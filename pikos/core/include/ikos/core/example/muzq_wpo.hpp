#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/iterator/transform_iterator.hpp>

#include <ikos/core/domain/domain_product.hpp>
#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/domain/numeric/operator.hpp>
#include <ikos/core/fixpoint/concurrent_fwd_fixpoint_iterator.hpp>
#include <ikos/core/linear_constraint.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/number/q_number.hpp>
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/semantic/graph.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/support/cast.hpp>

#include <tbb/concurrent_unordered_map.h>

namespace ikos {
namespace core {
namespace muzq {

/// \brief Base class for statements
template < typename VariableRef >
class Statement {
public:
  static_assert(
      IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");

public:
  enum StatementKind {
    ZLinearAssignmentKind,
    QLinearAssignmentKind,
    ZBinaryOperationKind,
    QBinaryOperationKind,
    ZLinearAssertionKind,
    QLinearAssertionKind,
    CheckPointKind,
  };

protected:
  // Kind of statement
  StatementKind _kind;

protected:
  /// \brief Protected constructor
  explicit Statement(StatementKind kind) : _kind(kind) {}

public:
  /// \brief Deleted copy constructor
  Statement(const Statement&) = delete;

  /// \brief Deleted move constructor
  Statement(Statement&&) = delete;

  /// \brief Deleted copy assignment operator
  Statement& operator=(const Statement&) = delete;

  /// \brief Deleted move assignment operator
  Statement& operator=(Statement&&) = delete;

  /// \brief Destructor
  virtual ~Statement() = default;

  /// \brief Get the kind of statement
  StatementKind kind() const { return this->_kind; }

  /// \brief Dump the statement for debugging purpose
  virtual void dump(std::ostream&) const = 0;

}; // end class Statement

/// \brief Linear assignment on integers
template < typename VariableRef >
class ZLinearAssignment final : public Statement< VariableRef > {
public:
  using StatementT = Statement< VariableRef >;
  using LinearExpressionT = LinearExpression< ZNumber, VariableRef >;

private:
  VariableRef _result;
  LinearExpressionT _operand;

public:
  /// \brief Create a linear assignment
  ZLinearAssignment(VariableRef result, LinearExpressionT operand)
      : StatementT(StatementT::ZLinearAssignmentKind),
        _result(result),
        _operand(std::move(operand)) {}

  /// \brief Return the result variable
  VariableRef result() const { return this->_result; }

  /// \brief Return the operand
  const LinearExpressionT& operand() const { return this->_operand; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const StatementT* s) {
    return s->kind() == StatementT::ZLinearAssignmentKind;
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream& o) const override {
    DumpableTraits< VariableRef >::dump(o, this->_result);
    o << " = ";
    this->_operand.dump(o);
  }

}; // end class ZLinearAssignment

/// \brief Linear assignment on rationals
template < typename VariableRef >
class QLinearAssignment final : public Statement< VariableRef > {
public:
  using StatementT = Statement< VariableRef >;
  using LinearExpressionT = LinearExpression< QNumber, VariableRef >;

private:
  VariableRef _result;
  LinearExpressionT _operand;

public:
  /// \brief Create a linear assignment
  QLinearAssignment(VariableRef result, LinearExpressionT operand)
      : StatementT(StatementT::QLinearAssignmentKind),
        _result(result),
        _operand(std::move(operand)) {}

  /// \brief Return the result variable
  VariableRef result() const { return this->_result; }

  /// \brief Return the operand
  const LinearExpressionT& operand() const { return this->_operand; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const StatementT* s) {
    return s->kind() == StatementT::QLinearAssignmentKind;
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream& o) const override {
    DumpableTraits< VariableRef >::dump(o, this->_result);
    o << " = ";
    this->_operand.dump(o);
  }

}; // end class QLinearAssignment

/// \brief Linear assignment on integers
template < typename VariableRef >
class ZBinaryOperation final : public Statement< VariableRef > {
public:
  using StatementT = Statement< VariableRef >;

private:
  VariableRef _result;
  numeric::BinaryOperator _op;
  VariableRef _left;
  VariableRef _right;

public:
  /// \brief Create a binary operation
  ZBinaryOperation(VariableRef result,
                   numeric::BinaryOperator op,
                   VariableRef left,
                   VariableRef right)
      : StatementT(StatementT::ZBinaryOperationKind),
        _result(result),
        _op(op),
        _left(left),
        _right(right) {}

  /// \brief Return the result variable
  VariableRef result() const { return this->_result; }

  /// \brief Return the binary operator
  numeric::BinaryOperator op() const { return this->_op; }

  /// \brief Return the left operand
  VariableRef left() const { return this->_left; }

  /// \brief Return the right operand
  VariableRef right() const { return this->_right; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const StatementT* s) {
    return s->kind() == StatementT::ZBinaryOperationKind;
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream& o) const override {
    DumpableTraits< VariableRef >::dump(o, this->_result);
    o << " = ";
    DumpableTraits< VariableRef >::dump(o, this->_left);
    o << " " << numeric::bin_operator_text(this->_op) << " ";
    DumpableTraits< VariableRef >::dump(o, this->_right);
  }

}; // end class ZBinaryOperation

/// \brief Linear assignment on rationals
template < typename VariableRef >
class QBinaryOperation final : public Statement< VariableRef > {
public:
  using StatementT = Statement< VariableRef >;

private:
  VariableRef _result;
  numeric::BinaryOperator _op;
  VariableRef _left;
  VariableRef _right;

public:
  /// \brief Create a binary operation
  QBinaryOperation(VariableRef result,
                   numeric::BinaryOperator op,
                   VariableRef left,
                   VariableRef right)
      : StatementT(StatementT::QBinaryOperationKind),
        _result(result),
        _op(op),
        _left(left),
        _right(right) {}

  /// \brief Return the result variable
  VariableRef result() const { return this->_result; }

  /// \brief Return the binary operator
  numeric::BinaryOperator op() const { return this->_op; }

  /// \brief Return the left operand
  VariableRef left() const { return this->_left; }

  /// \brief Return the right operand
  VariableRef right() const { return this->_right; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const StatementT* s) {
    return s->kind() == StatementT::QBinaryOperationKind;
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream& o) const override {
    DumpableTraits< VariableRef >::dump(o, this->_result);
    o << " = ";
    DumpableTraits< VariableRef >::dump(o, this->_left);
    o << " " << numeric::bin_operator_text(this->_op) << " ";
    DumpableTraits< VariableRef >::dump(o, this->_right);
  }

}; // end class QBinaryOperation

/// \brief Linear assertion on integers
template < typename VariableRef >
class ZLinearAssertion final : public Statement< VariableRef > {
public:
  using StatementT = Statement< VariableRef >;
  using LinearConstraintT = LinearConstraint< ZNumber, VariableRef >;

private:
  LinearConstraintT _cst;

public:
  /// \brief Create a linear assertion
  explicit ZLinearAssertion(LinearConstraintT cst)
      : StatementT(StatementT::ZLinearAssertionKind), _cst(std::move(cst)) {}

  const LinearConstraintT& constraint() const { return this->_cst; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const StatementT* s) {
    return s->kind() == StatementT::ZLinearAssertionKind;
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream& o) const override {
    o << "assert(";
    this->_cst.dump(o);
    o << ")";
  }

}; // end class ZLinearAssertion

/// \brief Linear assertion on rationals
template < typename VariableRef >
class QLinearAssertion final : public Statement< VariableRef > {
public:
  using StatementT = Statement< VariableRef >;
  using LinearConstraintT = LinearConstraint< QNumber, VariableRef >;

private:
  LinearConstraintT _cst;

public:
  /// \brief Create a linear assertion
  explicit QLinearAssertion(LinearConstraintT cst)
      : StatementT(StatementT::QLinearAssertionKind), _cst(std::move(cst)) {}

  const LinearConstraintT& constraint() const { return this->_cst; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const StatementT* s) {
    return s->kind() == StatementT::QLinearAssertionKind;
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream& o) const override {
    o << "assert(";
    this->_cst.dump(o);
    o << ")";
  }

}; // end class QLinearAssertion

/// \brief Checkpoint
template < typename VariableRef >
class CheckPoint final : public Statement< VariableRef > {
public:
  using StatementT = Statement< VariableRef >;

private:
  std::string _name;

public:
  /// \brief Create a checkpoint statement
  explicit CheckPoint(std::string name)
      : StatementT(StatementT::CheckPointKind), _name(std::move(name)) {}

  /// \brief Return the checkpoint name
  const std::string& name() const { return this->_name; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const StatementT* s) {
    return s->kind() == StatementT::CheckPointKind;
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream& o) const override {
    o << "checkpoint(" << this->_name << ")";
  }

}; // end class CheckPoint

/// \brief Helper for vectors of unique_ptr
template < typename T >
struct VectorUniquePtrExposeRawPtr {
  T* operator()(const std::unique_ptr< T >& p) const { return p.get(); }
};

/// \brief Helper for maps of unique_ptr
template < typename K, typename T >
struct MapUniquePtrExposeRawPtr {
  T* operator()(const std::pair< const K, std::unique_ptr< T > >& p) const {
    return p.second.get();
  }
};

// Forward declaration
template < typename VariableRef >
class ControlFlowGraph;

/// \brief Basic block
///
/// Represents a node in the control flow graph
template < typename VariableRef >
class BasicBlock {
public:
  using StatementT = Statement< VariableRef >;
  using StatementIterator = boost::transform_iterator<
      VectorUniquePtrExposeRawPtr< StatementT >,
      typename std::vector< std::unique_ptr< StatementT > >::const_iterator >;
  using BasicBlockIterator =
      typename std::vector< BasicBlock* >::const_iterator;

private:
  // Name
  std::string _name;

  // List of statements
  std::vector< std::unique_ptr< StatementT > > _statements;

  // List of successor basic blocks
  std::vector< BasicBlock* > _successors;

  // List of predecessor basic blocks
  std::vector< BasicBlock* > _predecessors;

private:
  /// \brief Private constructor
  explicit BasicBlock(std::string name) : _name(std::move(name)) {}

public:
  /// \brief Deleted copy constructor
  BasicBlock(const BasicBlock&) = delete;

  /// \brief Deleted move constructor
  BasicBlock(BasicBlock&&) = delete;

  /// \brief Deleted copy assignment operator
  BasicBlock& operator=(const BasicBlock&) = delete;

  /// \brief Deleted move assignment operator
  BasicBlock& operator=(BasicBlock&&) = delete;

  /// \brief Destructor
  ~BasicBlock() = default;

  /// \brief Return the number of statements
  auto size() const { return this->_statements.size(); }

  /// \brief Return the name
  const std::string& name() const { return this->_name; }

  /// \brief Begin iterator over the statements
  StatementIterator begin() const {
    return boost::make_transform_iterator(this->_statements.cbegin(),
                                          VectorUniquePtrExposeRawPtr<
                                              StatementT >());
  }

  /// \brief End iterator over the statements
  StatementIterator end() const {
    return boost::make_transform_iterator(this->_statements.cend(),
                                          VectorUniquePtrExposeRawPtr<
                                              StatementT >());
  }

  /// \brief Add a statement at the end of the basic block
  void add(std::unique_ptr< StatementT > stmt) {
    this->_statements.emplace_back(std::move(stmt));
  }

  /// \brief Return the number of statements
  std::size_t num_statements() const { return this->_statements.size(); }

  /// \brief Return true if the basic block is empty
  bool empty() const { return this->_statements.empty(); }

  /// \brief Begin iterator over the successors
  BasicBlockIterator successor_begin() const {
    return this->_successors.cbegin();
  }

  /// \brief End iterator over the successors
  BasicBlockIterator successor_end() const { return this->_successors.cend(); }

  /// \brief Begin iterator over the predecessors
  BasicBlockIterator predecessor_begin() const {
    return this->_predecessors.cbegin();
  }

  /// \brief End iterator over the predecessors
  BasicBlockIterator predecessor_end() const {
    return this->_predecessors.cend();
  }

  /// \brief Is the given basic block a successor?
  bool is_successor(const BasicBlock* bb) const {
    return std::find(this->_successors.begin(), this->_successors.end(), bb) !=
           this->_successors.end();
  }

  /// \brief Add the given basic block as a successor
  void add_successor(BasicBlock* bb) {
    if (!this->is_successor(bb)) {
      this->_successors.push_back(bb);
      bb->_predecessors.push_back(this);
    }
  }

  /// \brief Dump the basic block for debugging purpose
  void dump(std::ostream& o) const {
    o << this->_name << ":\n";
    for (auto it = this->begin(), et = this->end(); it != et; ++it) {
      o << "  ";
      (*it)->dump(o);
      o << ";\n";
    }
    o << "--> [";
    for (auto it = this->successor_begin(), et = this->successor_end();
         it != et;) {
      o << (*it)->name();
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << "]\n";
  }

  // Friends
  friend class ControlFlowGraph< VariableRef >;

}; // end class BasicBlock

/// \brief Control Flow Graph
///
/// It represents a code in the muzq language.
template < typename VariableRef >
class ControlFlowGraph {
public:
  using BasicBlockT = BasicBlock< VariableRef >;
  using BasicBlockIterator = boost::transform_iterator<
      MapUniquePtrExposeRawPtr< std::string, BasicBlockT >,
      typename std::unordered_map<
          std::string,
          std::unique_ptr< BasicBlockT > >::const_iterator >;

private:
  // Map from name to basic block
  std::unordered_map< std::string, std::unique_ptr< BasicBlockT > > _blocks;

  // Entry point
  BasicBlockT* _entry;

public:
  /// \brief Create a control flow graph
  ///
  /// \param entry Name of the entry point
  explicit ControlFlowGraph(const std::string& entry) {
    this->_entry = this->get(entry);
  }

  /// \brief Return the entry point
  BasicBlockT* entry() const { return this->_entry; }

  /// \brief Begin iterator over the basic blocks
  BasicBlockIterator begin() const {
    return boost::make_transform_iterator(this->_blocks.cbegin(),
                                          MapUniquePtrExposeRawPtr<
                                              std::string,
                                              BasicBlockT >());
  }

  /// \brief End iterator over the basic blocks
  BasicBlockIterator end() const {
    return boost::make_transform_iterator(this->_blocks.cend(),
                                          MapUniquePtrExposeRawPtr<
                                              std::string,
                                              BasicBlockT >());
  }

  /// \brief Get or create the basic block with the given name
  BasicBlockT* get(const std::string& name) {
    auto it = this->_blocks.find(name);
    if (it != this->_blocks.end()) {
      return it->second.get();
    } else {
      auto bb = new BasicBlockT(name);
      this->_blocks.emplace(name, std::unique_ptr< BasicBlockT >(bb));
      return bb;
    }
  }

  /// \brief Dump the control flow graph for debugging purpose
  void dump(std::ostream& o) const {
    for (auto it = this->begin(), et = this->end(); it != et; ++it) {
      (*it)->dump(o);
      o << "\n";
    }
  }

  auto size() const {
    return _blocks.size();
  }

}; // end class ControlFlowGraph

/// \brief Apply a statement visitor on a statement
///
/// A statement visitor looks like:
///
/// \code{.cpp}
/// struct MyStatementVisitor {
///   using ResultType = int;
///
///   int operator()(ZLinearAssignment* s) { ... }
///   int operator()(QLinearAssignment* s) { ... }
///   int operator()(ZBinaryOperation* s) { ... }
///   int operator()(QBinaryOperation* s) { ... }
///   int operator()(ZLinearAssertion* s) { ... }
///   int operator()(QLinearAssertion* s) { ... }
///   int operator()(CheckPoint* s) { ... }
/// };
/// \endcode
template < typename Visitor, typename VariableRef >
typename Visitor::ResultType apply_visitor(Visitor& visitor,
                                           Statement< VariableRef >* s) {
  using StatementT = Statement< VariableRef >;

  switch (s->kind()) {
    case StatementT::ZLinearAssignmentKind:
      return visitor(cast< ZLinearAssignment< VariableRef > >(s));
    case StatementT::QLinearAssignmentKind:
      return visitor(cast< QLinearAssignment< VariableRef > >(s));
    case StatementT::ZBinaryOperationKind:
      return visitor(cast< ZBinaryOperation< VariableRef > >(s));
    case StatementT::QBinaryOperationKind:
      return visitor(cast< QBinaryOperation< VariableRef > >(s));
    case StatementT::ZLinearAssertionKind:
      return visitor(cast< ZLinearAssertion< VariableRef > >(s));
    case StatementT::QLinearAssertionKind:
      return visitor(cast< QLinearAssertion< VariableRef > >(s));
    case StatementT::CheckPointKind:
      return visitor(cast< CheckPoint< VariableRef > >(s));
    default:
      ikos_unreachable("unexpected statement");
  }
}

} // end namespace muzq
} // end namespace core
} // end namespace ikos

namespace ikos {
namespace core {

/// \brief Implement GraphTraits for ControlFlowGraph
template < typename VariableRef >
struct GraphTraits< muzq::ControlFlowGraph< VariableRef >* > {
  using GraphRef = muzq::ControlFlowGraph< VariableRef >*;
  using NodeRef = muzq::BasicBlock< VariableRef >*;
  using SuccessorNodeIterator =
      typename muzq::BasicBlock< VariableRef >::BasicBlockIterator;
  using PredecessorNodeIterator =
      typename muzq::BasicBlock< VariableRef >::BasicBlockIterator;

  static NodeRef entry(GraphRef cfg) { return cfg->entry(); }

  static SuccessorNodeIterator successor_begin(NodeRef bb) {
    return bb->successor_begin();
  }

  static SuccessorNodeIterator successor_end(NodeRef bb) {
    return bb->successor_end();
  }

  static PredecessorNodeIterator predecessor_begin(NodeRef bb) {
    return bb->predecessor_begin();
  }

  static PredecessorNodeIterator predecessor_end(NodeRef bb) {
    return bb->predecessor_end();
  }
};

} // end namespace core
} // end namespace ikos

namespace ikos {
namespace core {
namespace muzq {

/// \brief Fixpoint iterator on a ControlFlowGraph
template < typename VariableRef, typename ZNumDomain, typename QNumDomain >
class FixpointIterator final : public InterleavedCFwdFixpointIterator<
                                   ControlFlowGraph< VariableRef >*,
                                   DomainProduct2< ZNumDomain, QNumDomain > > {
private:
  static_assert(
      numeric::IsAbstractDomain< ZNumDomain, ZNumber, VariableRef >::value,
      "ZNumDomain must be a numeric abstract domain on ZNumber");
  static_assert(
      numeric::IsAbstractDomain< QNumDomain, QNumber, VariableRef >::value,
      "QNumDomain must be a numeric abstract domain on QNumber");

public:
  using BasicBlockT = BasicBlock< VariableRef >;
  using ControlFlowGraphT = ControlFlowGraph< VariableRef >;
  using AbstractDomain = DomainProduct2< ZNumDomain, QNumDomain >;
  using Parent =
      InterleavedCFwdFixpointIterator< ControlFlowGraphT*, AbstractDomain >;

public:
  using StatementT = Statement< VariableRef >;
  using ZLinearAssignmentT = ZLinearAssignment< VariableRef >;
  using QLinearAssignmentT = QLinearAssignment< VariableRef >;
  using ZBinaryOperationT = ZBinaryOperation< VariableRef >;
  using QBinaryOperationT = QBinaryOperation< VariableRef >;
  using ZLinearAssertionT = ZLinearAssertion< VariableRef >;
  using QLinearAssertionT = QLinearAssertion< VariableRef >;
  using CheckPointT = CheckPoint< VariableRef >;

private:
  // Invariant at checkpoints
  std::unordered_map< std::string, AbstractDomain > _checkpoints;

  // Iteration counts for each basic block
  tbb::concurrent_unordered_map< const BasicBlockT*, unsigned > _iterations;

public:
  /// \brief Create a fixpoint iterator on the given ControlFlowGraph
  explicit FixpointIterator(ControlFlowGraphT& cfg)
    : Parent(&cfg) {}

  /// \brief Compute the fixpoint
  void run() { Parent::run(AbstractDomain::top()); }

  /// \brief Return the invariant at the given checkpoint
  const AbstractDomain& checkpoint(const std::string& name) {
    auto it = this->_checkpoints.find(name);
    if (it != this->_checkpoints.end()) {
      return it->second;
    } else {
      auto res = this->_checkpoints.emplace(name, AbstractDomain::bottom());
      return res.first->second;
    }
  }

  /// \brief Return the iteration count of given basic block
  unsigned iteration(const BasicBlockT* bb) {
    return _iterations[bb];
  }

private:
  /// \brief Execution engine
  ///
  /// Helper to execute sequential statements
  struct ExecutionEngine {
  public:
    AbstractDomain inv;

    std::unordered_map< std::string, AbstractDomain >& checkpoints;

  public:
    using ResultType = void;

    void operator()(ZLinearAssignmentT* s) {
      inv.first().assign(s->result(), s->operand());
    }

    void operator()(QLinearAssignmentT* s) {
      inv.second().assign(s->result(), s->operand());
    }

    void operator()(ZBinaryOperationT* s) {
      inv.first().apply(s->op(), s->result(), s->left(), s->right());
    }

    void operator()(QBinaryOperationT* s) {
      inv.second().apply(s->op(), s->result(), s->left(), s->right());
    }

    void operator()(ZLinearAssertionT* s) { inv.first().add(s->constraint()); }

    void operator()(QLinearAssertionT* s) { inv.second().add(s->constraint()); }

    void operator()(CheckPointT* s) { checkpoints[s->name()] = inv; }

  }; // end class ExecutionEngine

public:
  /// \brief Semantic transformer for a node
  ///
  /// This method is called with the abstract value representing the state
  /// of the program upon entering the node. The method should return an
  /// abstract value representing the state of the program after the node.
  AbstractDomain analyze_node(BasicBlockT* bb, AbstractDomain inv) override {
    _iterations[bb]++;
    ExecutionEngine engine{std::move(inv), this->_checkpoints};
    for (StatementT* stmt : *bb) {
      apply_visitor(engine, stmt);
    }
    return engine.inv;
  }

  /// \brief Semantic transformer for an edge
  ///
  /// This method is called with the abstract value representing the state of
  /// the program after exiting the source node. The method should return an
  /// abstract value representing the state of the program after the edge, at
  /// the entry of the destination node.
  AbstractDomain analyze_edge(BasicBlockT* /*src*/,
                              BasicBlockT* /*dest*/,
                              AbstractDomain inv) override {
    return inv;
  }

  /// \brief Process the computed abstract value for a node
  ///
  /// This method is called when the fixpoint is reached, and with the abstract
  /// value representing the state of the program upon entering the node.
  void process_pre(BasicBlockT*, const AbstractDomain&) override {}

  /// \brief Process the computed abstract value for a node
  ///
  /// This method is called when the fixpoint is reached, and with the abstract
  /// value representing the state of the program after the node.
  void process_post(BasicBlockT*, const AbstractDomain&) override {}

public:
  /// \brief Dump the fixpoint for debugging purpose
  void dump(std::ostream& o) const {
    for (auto it = this->_checkpoints.begin(), et = this->_checkpoints.end();
         it != et;
         ++it) {
      o << "Invariant at " << it->first << ":\n";
      it->second.dump(o);
      o << "\n";
    }
  }

}; // end class FixpointIterator

} // end namespace muzq
} // end namespace core
} // end namespace ikos
