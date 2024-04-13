
#include "output_stream.hpp"

#include <atomic>
#include <cassert>
#include <cstdio>
#include <thread>

#include <Windows.h>

namespace we {

namespace {

// Set this to true to print out the allocation counter after each message.
// Allows quickly checking for an equal number of calls to new and delete.
constexpr bool print_allocs = false;

/// A simple (hopefully) class for posting messages to and then having a background thread call
/// fwrite for those messages.
///
/// Because we apparently like to live dangerously the message queue is implemented as a linked list that is added
/// using compare-exchange. The writer thread is then woken up.
///
/// The writer thread removes the entire queue at once using an exchange and takes ownership of it.
///
/// The writer thread then turns the linked list into a doubly linked list by filling in the node::next member.
///
/// After this the writer thread traverses the linked list using the node::next pointers, this writes messages
/// out in the order they were posted.
///
/// Finally the the writer thread traverses the linked list one final time, calling delete on each node as it goes.
/// Then it goes back to sleep.
struct writer_linked_list {
   writer_linked_list()
   {
      // Messages are nice and all but we shouldn't take CPU time away from more important stuff.
      SetThreadPriority(_thread.native_handle(), THREAD_PRIORITY_LOWEST);
      SetThreadDescription(_thread.native_handle(),
                           L"WorldEdit Message Thread");
   }

   ~writer_linked_list()
   {
      _thread.request_stop();

      // Post an empty message to wake up the thread.
      post("");

      if constexpr (print_allocs) {
         _thread.join();

         assert(_alloc_count == 0);
      }
   }

   void post(std::string msg) noexcept
   {
      node* new_node = new node{.message = std::move(msg)};

      if constexpr (print_allocs) ++_alloc_count;

      new_node->previous = _head.load();

      while (!_head.compare_exchange_weak(new_node->previous, new_node))
         ;

      _head.notify_one();
   }

private:
   void loop(std::stop_token stop) noexcept
   {
      while (not stop.stop_requested()) {
         _head.wait(nullptr);

         node* message_chain_tail = _head.exchange(nullptr);

         if (not message_chain_tail) continue;

         // This will end up with the pointer to the node that was posted first.
         node* message_chain_begin = message_chain_tail;

         // Fill in the next pointer of the nodes. This will turn it into a doubly linked list.
         for (node* message = message_chain_tail->previous; message != nullptr;
              message = message->previous) {
            message->next = message_chain_begin;
            message_chain_begin = message;
         }

         // Now traverse the nodes in the order they were posted.
         for (node* message = message_chain_begin; message != nullptr;
              message = message->next) {
            std::fwrite(message->message.data(), message->message.size(), 1, stdout);
         }

         // Traverse one more time to delete each node.
         for (node* message = message_chain_begin; message != nullptr;) {
            node* dying_message = message;

            message = dying_message->next;

            delete dying_message;

            if constexpr (print_allocs) --_alloc_count;
         }

         if constexpr (print_allocs) {
            fmt::println(stdout, "allocs: {}", _alloc_count.load());
         }
      }
   }

   struct node {
      node* previous = nullptr;
      node* next = nullptr;
      std::string message;
   };

   std::jthread _thread{[this](std::stop_token stop) { loop(std::move(stop)); }};
   std::atomic<node*> _head = nullptr;
   std::atomic_int64_t _alloc_count = 0;
};

struct standard_output_stream final : public output_stream {
   using output_stream::write;

   void write(std::string string) noexcept override
   {
      _writer.post(std::move(string));
   }

private:
   writer_linked_list _writer;
};

}

void output_stream::vwrite(fmt::string_view fmt, fmt::format_args args) noexcept
{
   write(fmt::vformat(fmt, args));
}

void null_output_stream::write([[maybe_unused]] std::string name) noexcept {}

auto make_async_output_stream_stdout() -> std::unique_ptr<output_stream>
{
   return std::make_unique<standard_output_stream>();
}

}
