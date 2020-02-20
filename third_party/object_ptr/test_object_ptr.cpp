#include "object_ptr.hpp"
#include <assert.h>
#include <type_traits>
#include <memory>

void test_object_ptr_default_constructs_to_null() {
    jss::object_ptr<int> ap;
    assert(ap.get() == nullptr);
}

void test_object_ptr_can_be_constructed_from_nullptr() {
    jss::object_ptr<int> ap(nullptr);
    assert(ap.get() == nullptr);

    jss::object_ptr<int> ap2= nullptr;
    assert(ap2.get() == nullptr);

    [](jss::object_ptr<int> ap3) { assert(ap3.get() == nullptr); }(nullptr);
}

void test_object_ptr_can_be_constructed_from_raw_pointer() {
    int x;
    jss::object_ptr<int> ap(&x);
    assert(ap.get() == &x);

    jss::object_ptr<int> ap2= &x;
    assert(ap2.get() == &x);

    [&](jss::object_ptr<int> ap3) { assert(ap3.get() == &x); }(&x);
}

void test_object_ptr_can_be_dereferenced() {
    int x;
    jss::object_ptr<int> ap(&x);
    static_assert((std::is_same<decltype(*ap), int &>::value));

    assert(&*ap == &x);
}

void test_object_ptr_has_operator_arrow() {
    struct X {
        int data;
    };

    X x{42};

    jss::object_ptr<X> ap(&x);
    static_assert((std::is_same<decltype(ap.operator->()), X *>::value));

    assert(ap.operator->() == &x);
    assert(ap->data == 42);
}

void test_object_ptr_can_be_converted_to_bool() {
    int x;
    jss::object_ptr<int> ap(&x);
    assert(static_cast<bool>(ap));

    jss::object_ptr<int> ap2;
    assert(!static_cast<bool>(ap2));

    jss::object_ptr<int> ap3(nullptr);
    assert(!static_cast<bool>(ap3));
}

void test_object_ptr_can_be_copied() {
    class X {};

    static_assert(
        std::is_nothrow_copy_constructible<jss::object_ptr<X>>::value);

    X x;

    jss::object_ptr<X> ap(&x);
    jss::object_ptr<X> ap2(ap);

    assert(ap.get() == &x);
    assert(ap2.get() == &x);
    assert(ap);
    assert(ap2);
}

void test_object_ptr_can_be_assigned() {
    class X {};

    static_assert(std::is_nothrow_copy_assignable<jss::object_ptr<X>>::value);

    X x;

    jss::object_ptr<X> ap(&x);
    jss::object_ptr<X> ap2;

    assert(!ap2);

    ap2= ap;

    assert(ap.get() == &x);
    assert(ap2.get() == &x);
    assert(ap);
    assert(ap2);
}

void test_object_ptr_can_be_swapped() {
    class Y {};
    Y y1, y2;

    jss::object_ptr<Y> ap1(&y1), ap2(&y2);

    using std::swap;
    swap(ap1, ap2);

    assert(ap1.get() == &y2);
    assert(ap2.get() == &y1);

    swap(ap1, ap2);
    assert(ap1.get() == &y1);
    assert(ap2.get() == &y2);
}

void test_object_ptr_equality() {
    class Z {};

    Z z1, z2;

    jss::object_ptr<Z> ap1(&z1), ap2(&z2);

    static_assert((std::is_same<decltype(ap1 == ap2), bool>::value));
    assert(ap1 != ap2);
    assert(!(ap1 == ap2));

    ap1= ap2;

    assert(ap1 == ap2);
    assert(!(ap1 != ap2));

    assert(ap1 != nullptr);
    assert(nullptr != ap1);
    assert(!(ap1 == nullptr));
    assert(!(nullptr == ap1));

    ap2= nullptr;

    assert(ap2 == nullptr);
    assert(nullptr == ap2);
    assert(!(ap2 != nullptr));
    assert(!(nullptr != ap2));
}

void test_object_ptr_can_be_reset() {
    class A {};

    A a;

    jss::object_ptr<A> ap;

    ap.reset(&a);
    assert(ap.get() == &a);
    assert(&*ap == &a);

    ap.reset();
    assert(ap.get() == nullptr);

    ap.reset(&a);
    assert(ap.get() == &a);
    ap.reset(nullptr);
    assert(ap.get() == nullptr);
}

void test_object_ptr_has_not_operator() {
    int x;
    jss::object_ptr<int> ap(&x);
    static_assert(std::is_same<decltype(!ap), bool>::value);
    assert(!!ap);

    jss::object_ptr<int> ap2;
    assert(!ap2);

    jss::object_ptr<int> ap3(nullptr);
    assert(!ap3);
}

void test_object_ptr_has_ordering_comparisons() {
    int x[2];

    jss::object_ptr<int> ap1(&x[0]), ap2(&x[1]);

    assert(ap1 < ap2);
    assert(ap1 <= ap2);
    assert(!(ap2 < ap1));
    assert(!(ap2 <= ap1));
    assert(ap2 > ap1);
    assert(ap2 >= ap1);
    assert(!(ap1 > ap2));
    assert(!(ap1 >= ap2));

    jss::object_ptr<int> ap3(ap1);

    assert(!(ap1 < ap3));
    assert(!(ap3 < ap1));
    assert(ap1 <= ap3);
    assert(ap3 <= ap1);
    assert(!(ap1 > ap3));
    assert(!(ap3 > ap1));
    assert(ap1 >= ap3);
    assert(ap3 >= ap1);

    jss::object_ptr<int> np;

    assert((np < ap1) != (ap1 < np));
    assert((np > ap1) != (ap1 > np));
    assert((np < ap1) == (ap1 > np));
    assert((np > ap1) == (ap1 < np));
    assert((np <= ap1) == (ap1 >= np));
    assert((np >= ap1) == (ap1 <= np));
    assert((np >= ap1) == (ap1 < np));
    assert((np <= ap1) == (ap1 > np));
}

void test_object_ptr_implicit_conversions() {
    class Base {};
    class Derived : public Base {};

    Derived d;

    jss::object_ptr<Base> ap(&d);

    assert(ap.get() == &d);

    jss::object_ptr<Derived> ap2(&d);
    jss::object_ptr<Base> ap3(ap2);

    assert(ap3.get() == &d);
    assert(ap3 == ap2);

    ap3.reset();
    ap3= ap2;

    assert(ap3.get() == &d);
    assert(ap3 == ap2);
}

void test_object_ptr_can_be_explicitly_converted_to_raw_pointer() {
    int x;
    jss::object_ptr<int> ap(&x);

    assert(static_cast<int *>(ap) == &x);

    static_assert(!std::is_convertible<jss::object_ptr<int>, int *>::value);
}

void test_object_ptr_has_hash() {
    class X {};

    X x;

    jss::object_ptr<X> ap(&x);

    assert(std::hash<jss::object_ptr<X>>()(ap) == std::hash<X *>()(&x));
}

void test_object_ptr_can_be_constructed_from_shared_ptr() {
    auto ptr= std::make_shared<int>();
    jss::object_ptr<int> ap(ptr);
    assert(ap.get() == ptr.get());

    jss::object_ptr<int> ap2= ptr;
    assert(ap2.get() == ptr.get());

    [&](jss::object_ptr<int> ap3) { assert(ap3.get() == ptr.get()); }(ptr);
}

void test_object_ptr_can_be_constructed_from_unique_ptr() {
    auto ptr= std::make_unique<int>();
    jss::object_ptr<int> ap(ptr);
    assert(ap.get() == ptr.get());

    jss::object_ptr<int> ap2= ptr;
    assert(ap2.get() == ptr.get());

    [&](jss::object_ptr<int> ap3) { assert(ap3.get() == ptr.get()); }(ptr);
}

class Base {};
class Derived : public Base {};

void test_object_ptr_can_be_constructed_from_derived_pointer() {
    Derived d;
    jss::object_ptr<Base> ptr(&d);

    assert(ptr.get() == &d);
    assert(ptr == &d);

    jss::object_ptr<Base> ptr2= &d;

    assert(ptr2.get() == &d);

    [&](jss::object_ptr<Base> ap3) { assert(ap3.get() == &d); }(&d);
}

void test_object_ptr_can_be_constructed_from_shared_ptr_to_derived() {
    auto ptr= std::make_shared<Derived>();
    jss::object_ptr<Base> ap(ptr);
    assert(ap.get() == ptr.get());

    jss::object_ptr<Base> ap2= ptr;
    assert(ap2.get() == ptr.get());

    [&](jss::object_ptr<Base> ap3) { assert(ap3.get() == ptr.get()); }(ptr);
}

void test_object_ptr_can_be_constructed_from_unique_ptr_to_derived() {
    auto ptr= std::make_unique<Derived>();
    jss::object_ptr<Base> ap(ptr);
    assert(ap.get() == ptr.get());

    jss::object_ptr<Base> ap2= ptr;
    assert(ap2.get() == ptr.get());

    [&](jss::object_ptr<Base> ap3) { assert(ap3.get() == ptr.get()); }(ptr);
}

void test_object_ptr_can_be_constructed_from_object_ptr_to_derived() {
    Derived d;
    jss::object_ptr<Derived> ptr= &d;
    jss::object_ptr<Base> ap(ptr);
    assert(ap.get() == ptr.get());

    jss::object_ptr<Base> ap2= ptr;
    assert(ap2.get() == ptr.get());

    [&](jss::object_ptr<Base> ap3) { assert(ap3.get() == ptr.get()); }(ptr);
}

void test_static_pointer_cast() {
    Derived d;

    jss::object_ptr<Base> p(&d);

    auto dp= std::static_pointer_cast<Derived>(p);

    static_assert(
        std::is_same<decltype(dp), jss::object_ptr<Derived>>::value,
        "Static pointer cast gives correct type");

    assert(dp.get() == &d);
}

struct DynamicBase {
    virtual ~DynamicBase() {}
};
struct DynamicDerived : DynamicBase {};

struct DynamicDerived2 : DynamicBase {};

void test_dynamic_pointer_cast() {
    DynamicDerived d;

    jss::object_ptr<DynamicBase> p(&d);

    auto dp= std::dynamic_pointer_cast<DynamicDerived>(p);

    static_assert(
        std::is_same<decltype(dp), jss::object_ptr<DynamicDerived>>::value,
        "Static pointer cast gives correct type");

    assert(dp.get() == &d);

    DynamicDerived2 d2;

    jss::object_ptr<DynamicBase> p2(&d2);

    auto dp2= std::dynamic_pointer_cast<DynamicDerived>(p2);

    static_assert(
        std::is_same<decltype(dp), jss::object_ptr<DynamicDerived>>::value,
        "Static pointer cast gives correct type");

    assert(dp2.get() == nullptr);
}

struct ptr_chosen
{
    char dummy;
};

struct generic_chosen
{
    ptr_chosen dummy[ 2 ];
};

ptr_chosen sfinae_test( jss::object_ptr<int> )
{
    return {};
}

generic_chosen sfinae_test( ... )
{
    return {};
}

void test_sfinae()
{
    class NonSmartPointer
    {
    };

    assert( sizeof( sfinae_test( NonSmartPointer() ) ) == sizeof( generic_chosen ) );
}


int main() {
    test_object_ptr_default_constructs_to_null();
    test_object_ptr_can_be_constructed_from_nullptr();
    test_object_ptr_can_be_constructed_from_raw_pointer();
    test_object_ptr_can_be_dereferenced();
    test_object_ptr_has_operator_arrow();
    test_object_ptr_can_be_converted_to_bool();
    test_object_ptr_can_be_copied();
    test_object_ptr_can_be_assigned();
    test_object_ptr_can_be_swapped();
    test_object_ptr_equality();
    test_object_ptr_can_be_reset();
    test_object_ptr_has_not_operator();
    test_object_ptr_has_ordering_comparisons();
    test_object_ptr_implicit_conversions();
    test_object_ptr_can_be_explicitly_converted_to_raw_pointer();
    test_object_ptr_has_hash();
    test_object_ptr_can_be_constructed_from_shared_ptr();
    test_object_ptr_can_be_constructed_from_unique_ptr();
    test_object_ptr_can_be_constructed_from_derived_pointer();
    test_object_ptr_can_be_constructed_from_shared_ptr_to_derived();
    test_object_ptr_can_be_constructed_from_unique_ptr_to_derived();
    test_object_ptr_can_be_constructed_from_object_ptr_to_derived();
    test_static_pointer_cast();
    test_dynamic_pointer_cast();
    test_sfinae();
}
