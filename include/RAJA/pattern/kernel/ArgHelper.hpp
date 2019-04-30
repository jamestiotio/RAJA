/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file for kernel lambda executor.
 *
 ******************************************************************************
 */


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-19, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For details about use and distribution, please read RAJA/LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//


#ifndef RAJA_pattern_kernel_arghelper_HPP
#define RAJA_pattern_kernel_arghelper_HPP

#include "RAJA/config.hpp"

#include <iostream>
#include <type_traits>

#include "camp/camp.hpp"
#include "camp/concepts.hpp"
#include "camp/tuple.hpp"

#include "RAJA/util/macros.hpp"
#include "RAJA/util/types.hpp"

#include "RAJA/pattern/kernel/OffSet.hpp"
#include "RAJA/pattern/kernel/Param.hpp"
#include "RAJA/pattern/kernel/Seg.hpp"

namespace RAJA
{

namespace internal
{

using RAJA::statement::SegList;
using RAJA::statement::Seg;

using RAJA::statement::OffSet;
using RAJA::statement::OffSetList;

using RAJA::statement::ParamList;
using RAJA::statement::Param;

//Extracts arguments from segments, and parameters
template<typename Head, typename...Tail>
struct extractor
{};

template<camp::idx_t id, typename...Tail>
struct extractor<RAJA::statement::OffSet<id>, Tail...>
{
  template<typename Data>
  RAJA_HOST_DEVICE
  static auto extract_arg(Data &&data) ->
    decltype(camp::get<id>(data.offset_tuple))
  {
    return camp::get<id>(data.offset_tuple);
  }
};

template<camp::idx_t id, typename...Tail>
struct extractor<RAJA::statement::Seg<id>, Tail...>
{
  template<typename Data>
  RAJA_HOST_DEVICE
  static auto extract_arg(Data &&data) ->
    decltype(camp::get<id>(data.segment_tuple).begin()[camp::get<id>(data.offset_tuple)])
  {
    return camp::get<id>(data.segment_tuple).begin()[camp::get<id>(data.offset_tuple)];
  }
};

template<camp::idx_t id, typename...Tail>
struct extractor<RAJA::statement::Param<id>, Tail...>
{
  template<typename Data>
  RAJA_HOST_DEVICE
  static auto extract_arg(Data &&data)->
    decltype(camp::get<id>(data.param_tuple))
  {
    return camp::get<id>(data.param_tuple);
  }
};

//Helper to concatante list [Move to camp?]
template<typename ListA, typename ListB>
struct catList
{};

template<typename...itemsA, typename...itemsB>
struct catList<camp::list<itemsA...>, camp::list<itemsB...>> {

  RAJA_HOST_DEVICE
  static auto makeList(camp::list<itemsA...> const &, camp::list<itemsB...> const &) ->
    camp::list<itemsA...,itemsB...>
  {
    return camp::list<itemsA...,itemsB...> {};
  }

};

template<typename...itemsA>
struct catList<camp::list<itemsA...>, camp::list<>> {

  RAJA_HOST_DEVICE
  static auto makeList(camp::list<itemsA...> const &, camp::list<> const &) ->
    camp::list<itemsA...>
  {
    return camp::list<itemsA...> {};
  }

};

template<typename Arg>
struct listMaker
{
  RAJA_HOST_DEVICE
  static auto genList()
    -> camp::list<>
  {
    return camp::list<>{};
  }
};

//Converts SegList<1,2,3> -> list<Seg<0>, Seg<1>, Seg<2>>
template<camp::idx_t head, camp::idx_t... tail>
struct listMaker<SegList<head,tail...>>
{
  RAJA_HOST_DEVICE
  static auto genList()
    -> decltype(catList<camp::list<Seg<head>>,
            decltype(listMaker<SegList<tail...>>::genList())>::makeList(
            camp::list<Seg<head>>{},
            listMaker<SegList<tail...>>::genList()))
  {

    return catList<camp::list<Seg<head>>,
            decltype(listMaker<SegList<tail...>>::genList())>::makeList(
            camp::list<Seg<head>>{},
            listMaker<SegList<tail...>>::genList());
  }
};

//Converts Seg<id> -> list<Seg<id>>
template<camp::idx_t id>
struct listMaker<Seg<id>>
{
  RAJA_HOST_DEVICE
  static auto genList()
    -> camp::list<Seg<id>>
  {
    return camp::list<Seg<id>>{};
  }
};

//Converts OffSet<id> -> list<OffSet<id>>
template<camp::idx_t id>
struct listMaker<OffSet<id>>
{
  RAJA_HOST_DEVICE
  static auto genList()
    -> camp::list<OffSet<id>>
  {
    return camp::list<OffSet<id>>{};
  }
};

//Converts OffSetList<1,2,3> -> list<OffSet<0>, OffSet<1>, OffSet<2>>
template<camp::idx_t head, camp::idx_t... tail>
struct listMaker<OffSetList<head,tail...>>
{
  RAJA_HOST_DEVICE
  static auto genList()
    -> decltype(catList<camp::list<OffSet<head>>,
            decltype(listMaker<OffSetList<tail...>>::genList())>::makeList(
            camp::list<OffSet<head>>{},
            listMaker<OffSetList<tail...>>::genList()))
  {

    return catList<camp::list<OffSet<head>>,
            decltype(listMaker<OffSetList<tail...>>::genList())>::makeList(
            camp::list<OffSet<head>>{},
            listMaker<OffSetList<tail...>>::genList());
  }
};

//Converts ParamList<1,2,3> -> list<Param<0>, Param<1>, Param<2>>
template<camp::idx_t head, camp::idx_t... tail>
struct listMaker<ParamList<head,tail...>>
{
  RAJA_HOST_DEVICE
  static auto genList()
    -> decltype(catList<camp::list<Param<head>>,
            decltype(listMaker<ParamList<tail...>>::genList())>::makeList(
            camp::list<Param<head>>{},
            listMaker<ParamList<tail...>>::genList()))
  {

    return catList<camp::list<Param<head>>,
            decltype(listMaker<ParamList<tail...>>::genList())>::makeList(
            camp::list<Param<head>>{},
            listMaker<ParamList<tail...>>::genList());
  }
};


//Converts Param<id> -> list<Param<id>>
template<camp::idx_t id>
struct listMaker<Param<id>>
{
  RAJA_HOST_DEVICE
  static auto genList()
    -> camp::list<Param<id>>
  {

    return camp::list<Param<id>>{};
  }
};

//Helper structs to parse through template arguments
//Takes a pre-processing step convert SegList, ParamList
//into a list of Seg and Param's
template<typename List>
struct parser{};

template<>
struct parser<camp::list<>>
{
  RAJA_HOST_DEVICE
  static auto checkArgs()
    -> camp::list<>
  {
    return camp::list<> {};
  }
};

template <typename Head, typename... Tail>
struct parser<camp::list<Head, Tail...>>
{

  RAJA_HOST_DEVICE
  static auto checkArgs()
    -> decltype(catList<decltype(listMaker<Head>::genList()),
                decltype(parser<camp::list<Tail...> >::checkArgs())>
                ::makeList(listMaker<Head>::genList(), parser<camp::list<Tail...> >::checkArgs()))
  {

    return catList<decltype(listMaker<Head>::genList()),
                   decltype(parser<camp::list<Tail...> >::checkArgs())>
      ::makeList(listMaker<Head>::genList(), parser<camp::list<Tail...> >::checkArgs());
  }
};

template<typename List>
struct call_extractor
{};

template<typename ...Args>
struct call_extractor<camp::list<Args...>>
{
  template<typename Data>
  RAJA_HOST_DEVICE
  static auto make_tuple(Data &&data)
    -> decltype(camp::make_tuple(extractor<Args>::extract_arg(data) ...))
  {
    return camp::make_tuple(extractor<Args>::extract_arg(data) ...);
  }
};

}  // namespace internal

}  // end namespace RAJA


#endif /* RAJA_pattern_kernel_HPP */
