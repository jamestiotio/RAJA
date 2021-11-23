#ifndef PROTO_NEW_REDUCE_SEQ_REDUCE_HPP
#define PROTO_NEW_REDUCE_SEQ_REDUCE_HPP



namespace detail {

  // Init
  template<typename EXEC_POL, typename OP, typename T>
  camp::concepts::enable_if< std::is_same< EXEC_POL, RAJA::seq_exec> >
  init(Reducer<OP, T>& red) {
    red.val = Reducer<OP,T>::op::identity();
  }
  // Combine
  template<typename EXEC_POL, typename OP, typename T>
  camp::concepts::enable_if< std::is_same< EXEC_POL, RAJA::seq_exec> >
  combine(Reducer<OP, T>& out, const Reducer<OP, T>& in) {
    out.val = typename Reducer<OP,T>::op{}(out.val, in.val);
  }
  // Resolve
  template<typename EXEC_POL, typename OP, typename T>
  camp::concepts::enable_if< std::is_same< EXEC_POL, RAJA::seq_exec> >
  resolve(Reducer<OP, T>& red) {
    *red.target = OP{}(red.val, *red.target);
  }

} //  namespace detail

#endif //  PROTO_NEW_REDUCE_SEQ_REDUCE_HPP
