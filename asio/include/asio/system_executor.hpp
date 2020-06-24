//
// system_executor.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_SYSTEM_EXECUTOR_HPP
#define ASIO_SYSTEM_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/memory.hpp"
#include "asio/execution.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

class system_context;

/// An executor that uses arbitrary threads.
/**
 * The system executor represents an execution context where functions are
 * permitted to run on arbitrary threads. When the blocking.never property is
 * established, the system executor will schedule the function to run on an
 * unspecified system thread pool. When either blocking.possibly or
 * blocking.always is established, the executor invokes the function
 * immediately.
 */
template <typename Blocking, typename Relationship, typename Allocator>
class basic_system_executor
{
public:
  /// Default constructor.
  basic_system_executor() ASIO_NOEXCEPT
    : allocator_(Allocator())
  {
  }

  /// Obtain an executor with the @c blocking.possibly property.
  basic_system_executor<execution::blocking_t::possibly_t,
      Relationship, Allocator>
  require(execution::blocking_t::possibly_t) const
  {
    return basic_system_executor<execution::blocking_t::possibly_t,
        Relationship, Allocator>(allocator_);
  }

  /// Obtain an executor with the @c blocking.always property.
  basic_system_executor<execution::blocking_t::always_t,
      Relationship, Allocator>
  require(execution::blocking_t::always_t) const
  {
    return basic_system_executor<execution::blocking_t::always_t,
        Relationship, Allocator>(allocator_);
  }

  /// Obtain an executor with the @c blocking.never property.
  basic_system_executor<execution::blocking_t::never_t,
      Relationship, Allocator>
  require(execution::blocking_t::never_t) const
  {
    return basic_system_executor<execution::blocking_t::never_t,
        Relationship, Allocator>(allocator_);
  }

  /// Obtain an executor with the @c relationship.continuation property.
  basic_system_executor<Blocking,
      execution::relationship_t::continuation_t, Allocator>
  require(execution::relationship_t::continuation_t) const
  {
    return basic_system_executor<Blocking,
        execution::relationship_t::continuation_t, Allocator>(allocator_);
  }

  /// Obtain an executor with the @c relationship.fork property.
  basic_system_executor<Blocking,
      execution::relationship_t::fork_t, Allocator>
  require(execution::relationship_t::fork_t) const
  {
    return basic_system_executor<Blocking,
        execution::relationship_t::fork_t, Allocator>(allocator_);
  }

  /// Obtain an executor with the specified @c allocator property.
  template <typename OtherAllocator>
  basic_system_executor<Blocking, Relationship, OtherAllocator>
  require(execution::allocator_t<OtherAllocator> a) const
  {
    return basic_system_executor<Blocking,
        Relationship, OtherAllocator>(a.value());
  }

  /// Obtain an executor with the default @c allocator property.
  basic_system_executor<Blocking, Relationship, std::allocator<void> >
  require(execution::allocator_t<void>) const
  {
    return basic_system_executor<Blocking,
        Relationship, std::allocator<void> >();
  }

  /// Query the current value of the @c mapping property.
  static ASIO_CONSTEXPR execution::mapping_t query(
      execution::mapping_t) ASIO_NOEXCEPT
  {
    return execution::mapping.thread;
  }

  /// Query the current value of the @c context property.
  static system_context& query(execution::context_t) ASIO_NOEXCEPT;

  /// Query the current value of the @c blocking property.
  static ASIO_CONSTEXPR execution::blocking_t query(
      execution::blocking_t) ASIO_NOEXCEPT
  {
    return Blocking();
  }

  /// Query the current value of the @c relationship property.
  static ASIO_CONSTEXPR execution::relationship_t query(
      execution::relationship_t) ASIO_NOEXCEPT
  {
    return Relationship();
  }

  /// Query the current value of the @c allocator property.
  template <typename OtherAllocator>
  ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<OtherAllocator>) const ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Query the current value of the @c allocator property.
  ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<void>) const ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Query the occupancy (recommended number of work items) for the system
  /// context.
  std::size_t query(execution::occupancy_t) const ASIO_NOEXCEPT;

  /// Compare two executors for equality.
  /**
   * Two executors are equal if they refer to the same underlying io_context.
   */
  friend bool operator==(const basic_system_executor&,
      const basic_system_executor&) ASIO_NOEXCEPT
  {
    return true;
  }

  /// Compare two executors for inequality.
  /**
   * Two executors are equal if they refer to the same underlying io_context.
   */
  friend bool operator!=(const basic_system_executor&,
      const basic_system_executor&) ASIO_NOEXCEPT
  {
    return false;
  }

  /// Oneway execution function.
  template <typename Function>
  void execute(ASIO_MOVE_ARG(Function) f) const
  {
    this->do_execute(ASIO_MOVE_CAST(Function)(f), Blocking());
  }

#if !defined(ASIO_NO_TS_EXECUTORS)
  /// Obtain the underlying execution context.
  system_context& context() const ASIO_NOEXCEPT;

  /// Inform the executor that it has some outstanding work to do.
  /**
   * For the system executor, this is a no-op.
   */
  void on_work_started() const ASIO_NOEXCEPT
  {
  }

  /// Inform the executor that some work is no longer outstanding.
  /**
   * For the system executor, this is a no-op.
   */
  void on_work_finished() const ASIO_NOEXCEPT
  {
  }

  /// Request the system executor to invoke the given function object.
  /**
   * This function is used to ask the executor to execute the given function
   * object. The function object will always be executed inside this function.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void dispatch(ASIO_MOVE_ARG(Function) f, const OtherAllocator& a) const;

  /// Request the system executor to invoke the given function object.
  /**
   * This function is used to ask the executor to execute the given function
   * object. The function object will never be executed inside this function.
   * Instead, it will be scheduled to run on an unspecified system thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void post(ASIO_MOVE_ARG(Function) f, const OtherAllocator& a) const;

  /// Request the system executor to invoke the given function object.
  /**
   * This function is used to ask the executor to execute the given function
   * object. The function object will never be executed inside this function.
   * Instead, it will be scheduled to run on an unspecified system thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void defer(ASIO_MOVE_ARG(Function) f, const OtherAllocator& a) const;
#endif // !defined(ASIO_NO_TS_EXECUTORS)

private:
  template <typename, typename, typename> friend class basic_system_executor;

  // Constructor used by require().
  basic_system_executor(const Allocator& a)
    : allocator_(a)
  {
  }

  /// Execution helper implementation for the possibly blocking property.
  template <typename Function>
  void do_execute(ASIO_MOVE_ARG(Function) f,
      execution::blocking_t::possibly_t) const;

  /// Execution helper implementation for the always blocking property.
  template <typename Function>
  void do_execute(ASIO_MOVE_ARG(Function) f,
      execution::blocking_t::always_t) const;

  /// Execution helper implementation for the never blocking property.
  template <typename Function>
  void do_execute(ASIO_MOVE_ARG(Function) f,
      execution::blocking_t::never_t) const;

  // The allocator used for execution functions.
  Allocator allocator_;
};

/// An executor that uses arbitrary threads.
/**
 * The system executor represents an execution context where functions are
 * permitted to run on arbitrary threads. When the blocking.never property is
 * established, the system executor will schedule the function to run on an
 * unspecified system thread pool. When either blocking.possibly or
 * blocking.always is established, the executor invokes the function
 * immediately.
 */
typedef basic_system_executor<execution::blocking_t::possibly_t,
    execution::relationship_t::fork_t, std::allocator<void> >
  system_executor;

#if !defined(GENERATING_DOCUMENTATION)

namespace execution {

#if !defined(ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

template <typename Blocking, typename Relationship, typename Allocator>
struct is_executor<
    asio::basic_system_executor<Blocking, Relationship, Allocator>
  > : true_type
{
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

} // namespace execution
namespace traits {

#if !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename Blocking, typename Relationship,
    typename Allocator, typename Function>
struct execute_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    Function
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef void result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

template <typename Blocking, typename Relationship, typename Allocator>
struct require_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::blocking_t::possibly_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::basic_system_executor<
      asio::execution::blocking_t::possibly_t,
      Relationship, Allocator> result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct require_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::blocking_t::always_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::basic_system_executor<
      asio::execution::blocking_t::always_t,
      Relationship, Allocator> result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct require_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::blocking_t::never_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::basic_system_executor<
      asio::execution::blocking_t::never_t,
      Relationship, Allocator> result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct require_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::relationship_t::fork_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::basic_system_executor<Blocking,
      asio::execution::relationship_t::fork_t,
      Allocator> result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct require_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::relationship_t::continuation_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::basic_system_executor<Blocking,
      asio::execution::relationship_t::continuation_t,
      Allocator> result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct require_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::allocator_t<void>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::basic_system_executor<Blocking,
      Relationship, std::allocator<void> > result_type;
};

template <typename Blocking, typename Relationship,
    typename Allocator, typename OtherAllocator>
struct require_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::allocator_t<OtherAllocator>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::basic_system_executor<Blocking,
      Relationship, OtherAllocator> result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename Blocking, typename Relationship,
    typename Allocator, typename Property>
struct query_static_constexpr_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::mapping_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::mapping_t::thread_t result_type;

  static ASIO_CONSTEXPR result_type value() ASIO_NOEXCEPT
  {
    return result_type();
  }
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename Blocking, typename Relationship,
    typename Allocator, typename Property>
struct query_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::blocking_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::blocking_t result_type;
};

template <typename Blocking, typename Relationship,
    typename Allocator, typename Property>
struct query_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::relationship_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::relationship_t result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct query_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::context_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::system_context& result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct query_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::allocator_t<void>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

template <typename Blocking, typename Relationship, typename Allocator>
struct query_member<
    asio::basic_system_executor<Blocking, Relationship, Allocator>,
    asio::execution::allocator_t<Allocator>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

} // namespace traits

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/impl/system_executor.hpp"

#endif // ASIO_SYSTEM_EXECUTOR_HPP
