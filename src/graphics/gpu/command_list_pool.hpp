#pragma once

#include <hresult_error.hpp>
#include <utility/com_ptr.hpp>

#include <mutex>

#include <boost/container/small_vector.hpp>
#include <d3d12.h>
#include <gsl/gsl>

namespace sk::graphics::gpu {

class command_list_pool {
public:
   using command_list_interface = ID3D12GraphicsCommandList5;

   explicit command_list_pool(const D3D12_COMMAND_LIST_TYPE type, ID3D12Device6& device)
      : _type{type}, _device{device}
   {
   }

   command_list_pool(const command_list_pool&) = delete;
   auto operator=(const command_list_pool&) -> command_list_pool& = delete;

   command_list_pool(command_list_pool&&) = delete;
   auto operator=(command_list_pool&& other) -> command_list_pool& = delete;

   auto aquire() -> gsl::owner<command_list_interface*>
   {
      {
         std::lock_guard lock{_mutex};

         if (not _free_lists.empty()) {
            using std::swap;

            swap(_free_lists.front(), _free_lists.back());

            auto* result = _free_lists.back().release();

            _free_lists.pop_back();

            return result;
         }
      }

      return new_command_list();
   }

   void free(gsl::owner<command_list_interface*> command_list)
   {
      std::lock_guard lock{_mutex};

      assert(command_list->GetType() == _type);

      _free_lists.emplace_back(command_list);
   }

private:
   auto new_command_list() -> gsl::owner<command_list_interface*>
   {
      utility::com_ptr<command_list_interface> command_list;

      throw_if_failed(
         _device.CreateCommandList1(0, _type, D3D12_COMMAND_LIST_FLAG_NONE,
                                    IID_PPV_ARGS(command_list.clear_and_assign())));

      return command_list.release();
   }

   ID3D12Device6& _device;
   D3D12_COMMAND_LIST_TYPE _type = D3D12_COMMAND_LIST_TYPE_DIRECT;

   const static int inplace_pool_size = 16;

   template<typename T>
   using small_pool_vector = boost::container::small_vector<T, inplace_pool_size>;

   std::mutex _mutex;
   small_pool_vector<utility::com_ptr<command_list_interface>> _free_lists;
};

}
