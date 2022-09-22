/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file for HIP statement executors.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-23, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//


#ifndef RAJA_policy_hip_kernel_ForICount_HPP
#define RAJA_policy_hip_kernel_ForICount_HPP

#include "RAJA/config.hpp"

#include "RAJA/policy/hip/kernel/internal.hpp"


namespace RAJA
{

namespace internal
{

/*
 * Executor for thread work sharing loop inside HipKernel.
 * Mapping directly from threadIdx.xyz to indices
 * Assigns the loop iterate to offset ArgumentId
 * Assigns the loop count to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename ... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
  Data,
  statement::ForICount<ArgumentId, ParamId, RAJA::hip_warp_direct,
                       EnclosedStmts ...>,
  Types>
  : public HipStatementExecutor<
    Data,
    statement::For<ArgumentId, RAJA::hip_warp_direct,
                   EnclosedStmts ...>, Types > {

  using Base = HipStatementExecutor<
          Data,
          statement::For<ArgumentId, RAJA::hip_warp_direct,
                         EnclosedStmts ...>, Types >;

  using typename Base::enclosed_stmts_t;
  using typename Base::diff_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    diff_t len = segment_length<ArgumentId>(data);
    diff_t i = get_hip_dim<0>(threadIdx);


    // assign thread id directly to offset
    data.template assign_offset<ArgumentId>(i);
    data.template assign_param<ParamId>(i);

    // execute enclosed statements if in bounds
    enclosed_stmts_t::exec(data, thread_active && (i<len));

  }
};


/*
 * Executor for thread work sharing loop inside HipKernel.
 * Mapping directly from threadIdx.xyz to indices
 * Assigns the loop iterate to offset ArgumentId
 * Assigns the loop count to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename ... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
  Data,
  statement::ForICount<ArgumentId, ParamId, RAJA::hip_warp_loop,
                       EnclosedStmts ...>, Types >
  : public HipStatementExecutor<
    Data,
    statement::For<ArgumentId, RAJA::hip_warp_loop,
                   EnclosedStmts ...>, Types > {

  using Base = HipStatementExecutor<
          Data,
          statement::For<ArgumentId, RAJA::hip_warp_loop,
                         EnclosedStmts ...>, Types >;

  using typename Base::enclosed_stmts_t;
  using typename Base::diff_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    // block stride loop
    diff_t len = segment_length<ArgumentId>(data);
    diff_t i_init = threadIdx.x;
    diff_t i_stride = RAJA::policy::hip::WARP_SIZE;

    // Iterate through grid stride of chunks
    for (diff_t ii = 0; ii < len; ii += i_stride) {
      diff_t i = ii + i_init;

      // execute enclosed statements if any thread will
      // but mask off threads without work
      bool have_work = i < len;

      // Assign the x thread to the argument
      data.template assign_offset<ArgumentId>(i);
      data.template assign_param<ParamId>(i);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active && have_work);
    }
  }
};


/*
 * Executor for thread work sharing loop inside HipKernel.
 * Mapping directly from a warp lane
 * Assigns the loop index to offset ArgumentId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename Mask,
          typename ... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
  Data,
  statement::ForICount<ArgumentId, ParamId,
                       RAJA::hip_warp_masked_direct<Mask>,
                       EnclosedStmts ...>, Types >
  : public HipStatementExecutor<
    Data,
    statement::For<ArgumentId, RAJA::hip_warp_masked_direct<Mask>,
                   EnclosedStmts ...>, Types > {

  using Base = HipStatementExecutor<
          Data,
          statement::For<ArgumentId, RAJA::hip_warp_masked_direct<Mask>,
                         EnclosedStmts ...>, Types >;

  using typename Base::diff_t;

  using stmt_list_t = StatementList<EnclosedStmts ...>;

  // Set the argument type for this loop
  using NewTypes = setSegmentTypeFromData<Types, ArgumentId, Data>;

  using enclosed_stmts_t =
          HipStatementListExecutor<Data, stmt_list_t, NewTypes>;

  using mask_t = Mask;

  static_assert(mask_t::max_masked_size <= RAJA::policy::hip::WARP_SIZE,
                "BitMask is too large for HIP warp size");

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    diff_t len = segment_length<ArgumentId>(data);

    diff_t i = mask_t::maskValue((diff_t)threadIdx.x);

    // assign thread id directly to offset
    data.template assign_offset<ArgumentId>(i);
    data.template assign_param<ParamId>(i);

    // execute enclosed statements if in bounds
    enclosed_stmts_t::exec(data, thread_active && (i<len));
  }

};


/*
 * Executor for thread work sharing loop inside HipKernel.
 * Mapping directly from a warp lane
 * Assigns the loop index to offset ArgumentId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename Mask,
          typename ... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
  Data,
  statement::ForICount<ArgumentId, ParamId,
                       RAJA::hip_warp_masked_loop<Mask>,
                       EnclosedStmts ...>, Types >
  : public HipStatementExecutor<
    Data,
    statement::For<ArgumentId, RAJA::hip_warp_masked_loop<Mask>,
                   EnclosedStmts ...>, Types > {

  using Base = HipStatementExecutor<
          Data,
          statement::For<ArgumentId, RAJA::hip_warp_masked_loop<Mask>,
                         EnclosedStmts ...>, Types >;

  using typename Base::diff_t;

  using stmt_list_t = StatementList<EnclosedStmts ...>;

  // Set the argument type for this loop
  using NewTypes = setSegmentTypeFromData<Types, ArgumentId, Data>;

  using enclosed_stmts_t =
          HipStatementListExecutor<Data, stmt_list_t, NewTypes>;

  using mask_t = Mask;

  static_assert(mask_t::max_masked_size <= RAJA::policy::hip::WARP_SIZE,
                "BitMask is too large for HIP warp size");

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    // masked size strided loop
    diff_t len = segment_length<ArgumentId>(data);
    diff_t i_init = mask_t::maskValue((diff_t)threadIdx.x);
    diff_t i_stride = (diff_t) mask_t::max_masked_size;

    // Iterate through grid stride of chunks
    for (diff_t ii = 0; ii < len; ii += i_stride) {
      diff_t i = ii + i_init;

      // execute enclosed statements if any thread will
      // but mask off threads without work
      bool have_work = i < len;

      // Assign the x thread to the argument and param
      data.template assign_offset<ArgumentId>(i);
      data.template assign_param<ParamId>(i);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active && have_work);
    }
  }

};


/*
 * Executor for thread work sharing loop inside HipKernel.
 * Mapping directly from a warp lane
 * Assigns the loop index to offset ArgumentId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename Mask,
          typename ... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
  Data,
  statement::ForICount<ArgumentId, ParamId,
                       RAJA::hip_thread_masked_direct<Mask>,
                       EnclosedStmts ...>, Types >
  : public HipStatementExecutor<
    Data,
    statement::For<ArgumentId, RAJA::hip_thread_masked_direct<Mask>,
                   EnclosedStmts ...>, Types > {

  using Base = HipStatementExecutor<
          Data,
          statement::For<ArgumentId, RAJA::hip_thread_masked_direct<Mask>,
                         EnclosedStmts ...>, Types >;

  using typename Base::diff_t;

  using stmt_list_t = StatementList<EnclosedStmts ...>;

  // Set the argument type for this loop
  using NewTypes = setSegmentTypeFromData<Types, ArgumentId, Data>;

  using enclosed_stmts_t =
          HipStatementListExecutor<Data, stmt_list_t, NewTypes>;

  using mask_t = Mask;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    diff_t len = segment_length<ArgumentId>(data);

    diff_t i = mask_t::maskValue((diff_t)threadIdx.x);

    // assign thread id directly to offset
    data.template assign_offset<ArgumentId>(i);
    data.template assign_param<ParamId>(i);

    // execute enclosed statements if in bounds
    enclosed_stmts_t::exec(data, thread_active && (i<len));
  }

};


/*
 * Executor for thread work sharing loop inside HipKernel.
 * Mapping directly from a warp lane
 * Assigns the loop index to offset ArgumentId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename Mask,
          typename ... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
  Data,
  statement::ForICount<ArgumentId, ParamId,
                       RAJA::hip_thread_masked_loop<Mask>,
                       EnclosedStmts ...>, Types >
  : public HipStatementExecutor<
    Data,
    statement::For<ArgumentId, RAJA::hip_thread_masked_loop<Mask>,
                   EnclosedStmts ...>, Types > {

  using Base = HipStatementExecutor<
          Data,
          statement::For<ArgumentId, RAJA::hip_thread_masked_loop<Mask>,
                         EnclosedStmts ...>, Types >;

  using typename Base::diff_t;

  using stmt_list_t = StatementList<EnclosedStmts ...>;

  // Set the argument type for this loop
  using NewTypes = setSegmentTypeFromData<Types, ArgumentId, Data>;

  using enclosed_stmts_t =
          HipStatementListExecutor<Data, stmt_list_t, NewTypes>;

  using mask_t = Mask;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    // masked size strided loop
    diff_t len = segment_length<ArgumentId>(data);
    diff_t i_init = mask_t::maskValue((diff_t)threadIdx.x);
    diff_t i_stride = (diff_t) mask_t::max_masked_size;

    // Iterate through grid stride of chunks
    for (diff_t ii = 0; ii < len; ii += i_stride) {
      diff_t i = ii + i_init;

      // execute enclosed statements if any thread will
      // but mask off threads without work
      bool have_work = i < len;

      // Assign the x thread to the argument
      data.template assign_offset<ArgumentId>(i);
      data.template assign_param<ParamId>(i);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active && have_work);
    }
  }

};


/*
 * Executor for work sharing inside HipKernel.
 * Provides a direct mapping.
 * Assigns the loop index to offset ArgumentId
 * Assigns the loop index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename Indexer,
          typename... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
    Data,
    statement::ForICount<ArgumentId, ParamId, RAJA::internal::HipIndexDirect<Indexer>, EnclosedStmts...>,
    Types>
    : public HipStatementExecutor<
        Data,
        statement::For<ArgumentId, RAJA::internal::HipIndexDirect<Indexer>, EnclosedStmts...>,
        Types> {

  using Base = HipStatementExecutor<
      Data,
      statement::For<ArgumentId, RAJA::internal::HipIndexDirect<Indexer>, EnclosedStmts...>,
      Types>;

  using typename Base::enclosed_stmts_t;
  using typename Base::diff_t;

  static
  inline RAJA_DEVICE void exec(Data &data, bool thread_active)
  {
    // grid stride loop
    diff_t len = segment_length<ArgumentId>(data);
    diff_t i = Indexer::template index<diff_t>();

    // Assign the index to the argument and param
    data.template assign_offset<ArgumentId>(i);
    data.template assign_param<ParamId>(i);

    // execute enclosed statements
    enclosed_stmts_t::exec(data, thread_active && (i < len));
  }
};

/*
 * Executor for work sharing inside HipKernel.
 * Provides a strided loop.
 * Assigns the loop index to offset ArgumentId
 * Assigns the loop index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename Indexer,
          typename... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
    Data,
    statement::ForICount<ArgumentId, ParamId, RAJA::internal::HipIndexLoop<Indexer>, EnclosedStmts...>,
    Types>
    : public HipStatementExecutor<
        Data,
        statement::For<ArgumentId, RAJA::internal::HipIndexLoop<Indexer>, EnclosedStmts...>,
        Types> {

  using Base = HipStatementExecutor<
      Data,
      statement::For<ArgumentId, RAJA::internal::HipIndexLoop<Indexer>, EnclosedStmts...>,
      Types>;

  using typename Base::enclosed_stmts_t;
  using typename Base::diff_t;

  static
  inline RAJA_DEVICE void exec(Data &data, bool thread_active)
  {
    // grid stride loop
    diff_t len = segment_length<ArgumentId>(data);
    diff_t i_init = Indexer::template index<diff_t>();
    diff_t i_stride = Indexer::template size<diff_t>();

    // Iterate through chunks
    for (diff_t ii = 0; ii < len; ii += i_stride) {
      diff_t i = ii + i_init;

      // execute enclosed statements if any thread will
      // but mask off threads without work
      bool have_work = i < len;

      // Assign the index to the argument and param
      data.template assign_offset<ArgumentId>(i);
      data.template assign_param<ParamId>(i);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active && have_work);
    }
  }
};


/*
 * Executor for sequential loops inside of a HipKernel.
 *
 * This is specialized since it need to execute the loop immediately.
 * Assigns the loop index to offset ArgumentId
 * Assigns the loop index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename... EnclosedStmts,
          typename Types>
struct HipStatementExecutor<
    Data,
    statement::ForICount<ArgumentId, ParamId, seq_exec, EnclosedStmts...>, Types >
    : public HipStatementExecutor<
        Data,
        statement::For<ArgumentId, seq_exec, EnclosedStmts...>, Types > {

  using Base = HipStatementExecutor<
      Data,
      statement::For<ArgumentId, seq_exec, EnclosedStmts...>, Types >;

  using typename Base::enclosed_stmts_t;
  using typename Base::diff_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    diff_t len = segment_length<ArgumentId>(data);

    for(diff_t i = 0;i < len;++ i){
      // Assign i to the argument
      data.template assign_offset<ArgumentId>(i);
      data.template assign_param<ParamId>(i);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active);
    }
  }
};





}  // namespace internal
}  // end namespace RAJA


#endif /* RAJA_policy_hip_kernel_ForICount_HPP */
