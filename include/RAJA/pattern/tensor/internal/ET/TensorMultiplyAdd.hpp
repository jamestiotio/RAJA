/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   RAJA header file defining SIMD/SIMT register operations.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-23, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_pattern_tensor_ET_TensorMultiplyAddAdd_HPP
#define RAJA_pattern_tensor_ET_TensorMultiplyAddAdd_HPP

#include "RAJA/config.hpp"

#include "RAJA/util/macros.hpp"

#include "RAJA/pattern/tensor/internal/ET/ExpressionTemplateBase.hpp"
#include "RAJA/pattern/tensor/internal/ET/MultiplyOperator.hpp"

namespace RAJA
{
namespace internal
{
namespace expt
{


  namespace ET
  {


    /*!
     * Expression for LHS*RHS+ADD, which allows for accessing FMA style
     * operations.
     *
     * This ET can only be generated by contracting an Add and Multiple ET.
     *
     */
    template<typename LEFT_OPERAND_TYPE, typename RIGHT_OPERAND_TYPE, typename ADD_OPERAND_TYPE>
    class TensorMultiplyAdd : public TensorExpressionBase<TensorMultiplyAdd<LEFT_OPERAND_TYPE, RIGHT_OPERAND_TYPE, ADD_OPERAND_TYPE>> {
      public:
        using self_type = TensorMultiplyAdd<LEFT_OPERAND_TYPE, RIGHT_OPERAND_TYPE, ADD_OPERAND_TYPE>;
        using left_operand_type = LEFT_OPERAND_TYPE;
        using right_operand_type = RIGHT_OPERAND_TYPE;
        using add_operand_type = ADD_OPERAND_TYPE;
        using multiply_op = MultiplyOperator<LEFT_OPERAND_TYPE, RIGHT_OPERAND_TYPE>;

        using element_type = typename LEFT_OPERAND_TYPE::element_type;
        using index_type = typename LEFT_OPERAND_TYPE::index_type;

        using result_type = typename multiply_op::result_type;
        static constexpr camp::idx_t s_num_dims = multiply_op::s_num_dims;

      private:
        left_operand_type m_left_operand;
        right_operand_type m_right_operand;
        add_operand_type m_add_operand;

      public:
        RAJA_INLINE
        RAJA_HOST_DEVICE
        TensorMultiplyAdd(left_operand_type const &left_operand, right_operand_type const &right_operand,
                          add_operand_type const &add_operand) :
        m_left_operand{left_operand}, m_right_operand{right_operand}, m_add_operand{add_operand}
        {}


        template<typename TILE_TYPE>
        RAJA_INLINE
        RAJA_HOST_DEVICE
        auto eval(TILE_TYPE const &tile) const ->
          decltype(multiply_op::multiply_add(tile, m_left_operand, m_right_operand, m_add_operand))
        {
          return multiply_op::multiply_add(tile, m_left_operand, m_right_operand, m_add_operand);
        }


        RAJA_INLINE
        RAJA_HOST_DEVICE
        void print_ast() const {
          printf("MultiplyAdd[");
          multiply_op::print_ast();
          printf("](");
          m_left_operand.print_ast();
          printf(", ");
          m_right_operand.print_ast();
          printf(", ");
          m_add_operand.print_ast();
          printf(")");
        }



    };




  } // namespace ET

  } // namespace internal
} // namespace expt

}  // namespace RAJA


#endif
