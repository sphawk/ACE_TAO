// $Id$

// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    gen_bounded_sequence_ci.cpp
//
// = DESCRIPTION
//    Visitor generating code for Sequence in the client inline
//
// = AUTHOR
//    Michael Kircher
//
//    Modifications by Aniruddha Gokhale
//
// ============================================================================

//#include      "idl.h"
//#include      "idl_extern.h"
#include        "be.h"

#include "be_visitor_sequence.h"

ACE_RCSID(be_visitor_sequence, gen_bounded_sequence_ci, "$Id$")


int
be_visitor_sequence_ci::gen_bounded_sequence (be_sequence *node)
{
  TAO_OutStream *os = this->ctx_->stream ();
  be_type *bt;

  // retrieve the base type since we may need to do some code
  // generation for the base type.
  bt = be_type::narrow_from_decl (node->base_type ());
  if (!bt)
    {
      ACE_ERROR_RETURN ((LM_ERROR,
                         "(%N:%l) be_visitor_sequence_ci::"
                         "visit_sequence - "
                         "Bad element type\n"), -1);
    }

  // generate the class name
  be_type  *pt; // base types

  if (bt->node_type () == AST_Decl::NT_typedef)
    {
      // get the primitive base type of this typedef node
      be_typedef *t = be_typedef::narrow_from_decl (bt);
      pt = t->primitive_base_type ();
    }
  else
    pt = bt;

  const char * class_name = node->instance_name ();

  static char full_class_name [NAMEBUFSIZE];
  ACE_OS::memset (full_class_name, '\0', NAMEBUFSIZE);

  if (node->is_nested ())
    {
      ACE_OS::sprintf (full_class_name, "%s::%s",
                       be_scope::narrow_from_scope (node->defined_in ())->decl ()->fullname (),
                       class_name);
    }
  else
    {
      ACE_OS::sprintf (full_class_name, "%s",
                       class_name);
    }


  be_visitor_context ctx (*this->ctx_);
  ctx.state (TAO_CodeGen::TAO_SEQUENCE_BASE_CI);
  be_visitor *visitor = tao_cg->make_visitor (&ctx);

  // !! branching in either compile time template instantiation
  // or manual template instatiation
  os->gen_ifdef_AHETI();

  os->gen_ifdef_macro (class_name);

  os->indent ();

  // first generate the static methods since they are used by others. Sinc
  // ethey are inlined, their definition needs to come before their use else
  // some compilers give lost of warnings.

  // Static Operations
  // allocbuf
  *os << "// = Static operations." << be_nl
      << "ACE_INLINE "; 

  // the accept is here the first time used and if an 
  // error occurs, it will occur here. Later no check
  // for errors will be done.
  if (pt->accept (visitor) == -1)
    {
      ACE_ERROR_RETURN ((LM_ERROR,
                         "(%N:%l) be_visitor_sequence::"
                         "gen_bounded_sequence - "
                         "base type visit failed\n"),
                        -1);
    }
  *os << " *" << be_nl
      << full_class_name << "::allocbuf (CORBA::ULong) "
      << "// Allocate storage for the sequence." << be_nl
      << "{" << be_idt_nl
      << "return new "; 
  pt->accept (visitor); 
  *os << "[" << node->max_size () << "];" << be_uidt_nl
      << "}" << be_nl
      << be_nl;
  
  // free_buf
  *os << "ACE_INLINE void " << be_nl
      << full_class_name << "::freebuf ("; 
  pt->accept (visitor); 
  *os << " *buffer) // Free the sequence." << be_nl
      << "{" << be_idt_nl
      << "delete [] buffer;" << be_uidt_nl
      << "}" << be_nl
      << be_nl;
  
  // Constructor
  *os << "ACE_INLINE" << be_nl
      << full_class_name << "::" << class_name << " (void)" << be_nl
      << "// Default constructor." << be_nl
      << "  : TAO_Bounded_Base_Sequence (" << node->max_size () 
      << ", 0, " << full_class_name << "::allocbuf (" << node->max_size () 
      << "), 0)" << be_nl
      << "{" << be_nl
      << "}" << be_nl
      << be_nl;

  // constructor
  *os << "ACE_INLINE" << be_nl;
  *os << full_class_name << "::" << class_name << " (CORBA::ULong length," << be_idt_nl;
  pt->accept (visitor); 
  *os <<" *data," << be_nl
      << "CORBA::Boolean release)" << be_uidt_nl
      << "// Constructor using the data and memory management flag." << be_nl
      << "  : TAO_Bounded_Base_Sequence (" << node->max_size () << ", length, data, release)" << be_nl
      << "{" << be_nl
      << "  this->_allocate_buffer (" << node->max_size () << ");" << be_nl 
      << "}" << be_nl
      << be_nl;

  // constructor
  *os << "ACE_INLINE" << be_nl
      << full_class_name << "::" << class_name 
      << " (const " << full_class_name << " &rhs)" << be_nl
      << "// Copy constructor." << be_idt_nl
      << ": TAO_Bounded_Base_Sequence (rhs)" << be_uidt_nl
      << "{" << be_idt_nl;
  pt->accept(visitor); 
  *os <<" *tmp1 = " << full_class_name << "::allocbuf (" << node->max_size () 
      << ");" << be_nl
      << be_nl;
  pt->accept(visitor); 
  *os <<" * const tmp2 = ACE_reinterpret_cast (";
  pt->accept (visitor); 
  *os << " * ACE_CAST_CONST, rhs.buffer_);" << be_nl
      << be_nl
      << "for (CORBA::ULong i = 0; i < this->length_; ++i)" << be_idt_nl
      << "tmp1[i] = tmp2[i];" << be_uidt_nl
      << be_nl
      << "this->buffer_ = tmp1;" << be_uidt_nl
      << "}" << be_nl
      << be_nl;

  // operator=
  *os << "ACE_INLINE " << full_class_name << " &" << be_nl
      << full_class_name << "::operator= (const " << full_class_name << " &rhs)" << be_nl
      << "// Assignment operator.  " << be_nl
      << "{" << be_idt_nl
      << "if (this == &rhs)" << be_idt_nl
      << "return *this;" << be_uidt_nl
      << be_nl
      << "if (this->release_)" << be_nl
      << "{" << be_nl
      << "}" << be_nl
      << "else" << be_idt_nl
      << "this->buffer_ = " << full_class_name << "::allocbuf (rhs.maximum_);" << be_nl
      << be_nl
      <<"TAO_Bounded_Base_Sequence::operator= (rhs);" << be_nl
      << be_nl;
  pt->accept(visitor); 
  *os <<"* tmp1 = ACE_reinterpret_cast (";
  pt->accept(visitor); 
  *os << " *, this->buffer_);" << be_nl;
  pt->accept(visitor); 
  *os <<"* const tmp2 = ACE_reinterpret_cast (";
  pt->accept (visitor); 
  *os << " * ACE_CAST_CONST, rhs.buffer_);" << be_nl
      << be_nl
      << "for (CORBA::ULong i = 0; i < this->length_; ++i)" << be_idt_nl
      << "tmp1[i] = tmp2[i];" << be_uidt_nl
      << be_nl
      << "return *this;" << be_uidt_nl << be_uidt_nl
      << "}" << be_nl
      << be_nl;
  
  // Accessors
  *os << "// = Accessors." << be_nl;
  *os << "ACE_INLINE ";
  pt->accept(visitor); 
  *os << " &" << be_nl;
  *os << full_class_name << "::operator[] (CORBA::ULong i)// operator []" << be_nl
      << "{" << be_idt_nl
      << "ACE_ASSERT (i < this->maximum_);" << be_nl;
  pt->accept(visitor); 
  *os <<" *tmp = ACE_reinterpret_cast (";
  pt->accept (visitor); 
  *os << "*,this->buffer_);" << be_nl
      << "return tmp[i];" << be_uidt_nl
      << "}" << be_nl;

  *os << "ACE_INLINE const "; 
  pt->accept (visitor); 
  *os << " &" << be_nl
      << full_class_name << "::operator[] (CORBA::ULong i) const// operator []" << be_nl
      << "{" << be_idt_nl
      << "ACE_ASSERT (i < this->maximum_);" << be_nl
      << "const "; 
  pt->accept (visitor); 
  *os << "* tmp = ACE_reinterpret_cast (const ";
  pt->accept (visitor); 
  *os << "* ACE_CAST_CONST,this->buffer_);" << be_nl
      << "return tmp[i];" << be_uidt_nl
      << "}" << be_nl
      << be_nl;
  
  // get_buffer
  *os << "ACE_INLINE ";
  pt->accept(visitor); 
  *os << " *" << be_nl
      << full_class_name << "::get_buffer (CORBA::Boolean orphan)" << be_nl
      << "{" << be_idt_nl;
  pt->accept(visitor); 
  *os << " *result = 0;" << be_nl
      << "if (orphan == 0)" << be_nl
      << "{" << be_idt_nl
      << "// We retain ownership." << be_nl
      << "if (this->buffer_ == 0)" << be_nl
      << "{" << be_idt_nl
      << "result = " << full_class_name << "::allocbuf (this->maximum_);" << be_nl
      << "this->buffer_ = result;" << be_uidt_nl
      << "}" << be_nl
      << "else" << be_nl
      << "{" << be_idt_nl
      << "result = ACE_reinterpret_cast (";
  pt->accept (visitor);
  *os << "*, this->buffer_);" << be_uidt_nl
      << "}" << be_uidt_nl
      << "}" << be_nl
      << "else // if (orphan == 1)" << be_nl
      << "{" << be_idt_nl
      << "if (this->release_ != 0)" << be_nl
      << "{" << be_idt_nl
      << "// We set the state back to default and relinquish" << be_nl
      << "// ownership." << be_nl
      << "result = ACE_reinterpret_cast("; 
  pt->accept (visitor); 
  *os << "*,this->buffer_);" << be_nl
      << "this->maximum_ = 0;" << be_nl
      << "this->length_ = 0;" << be_nl
      << "this->buffer_ = 0;" << be_nl
      << "this->release_ = 0;" << be_uidt_nl
      << "}" << be_uidt_nl
      << "}" << be_nl
      << "return result;" << be_uidt_nl
      << "}" << be_nl
      << be_nl;
  
  // get_buffer
  *os << "ACE_INLINE const "; 
  pt->accept (visitor); 
  *os << " *" << be_nl
      << full_class_name << "::get_buffer (void) const" << be_nl
      << "{" << be_idt_nl
      << "return ACE_reinterpret_cast(const ";
  pt->accept (visitor);
  *os << " * ACE_CAST_CONST, this->buffer_);" << be_uidt_nl
      << "}" << be_nl
      << be_nl;
  
  // replace
  *os << "ACE_INLINE void " << be_nl
      << full_class_name << "::replace (CORBA::ULong max," << be_idt_nl
      << "CORBA::ULong length," << be_nl;
  pt->accept(visitor); 
  *os <<" *data," << be_nl
      << "CORBA::Boolean release)" << be_uidt_nl
      << "{" << be_idt_nl
      << "this->maximum_ = max;" << be_nl
      << "this->length_ = length;" << be_nl
      << "if (this->buffer_ && this->release_ == 1)" << be_nl
      << "{" << be_idt_nl;
  pt->accept(visitor); 
  *os <<"* tmp = ACE_reinterpret_cast(";
  pt->accept (visitor); 
  *os << "* ACE_CAST_CONST, this->buffer_);" << be_nl
      << full_class_name << "::freebuf (tmp);" << be_uidt_nl
      << "}" << be_nl
      << "this->buffer_ = data;" << be_nl
      << "this->release_ = release;" << be_uidt_nl
      << "}" << be_nl;
  
  os->gen_endif (); // endif macro
  
  // generate #endif for AHETI
  os->gen_endif_AHETI();
  
  delete visitor;
  return 0;
}
