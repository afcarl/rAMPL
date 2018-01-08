#ifndef GUARD_RAMPL_h
#define GUARD_RAMPL_h

#include <string>
#include "ampl/ampl.h"
#include "renvironment.h"
#include "rvar_entity.h"
#include "rcon_entity.h"
#include "robj_entity.h"
#include "rset_entity.h"
#include "rparam_entity.h"
#include <Rcpp.h>

class AMPLOutputHandler : public ampl::OutputHandler {
public:
  Rcpp::Function outputhandler;
  AMPLOutputHandler(Rcpp::Function _outputhandler): outputhandler(_outputhandler) { }
  void output(ampl::output::Kind kind, const char* output) {
    outputhandler(output);
  }
};

class AMPLErrorHandler : public ampl::ErrorHandler {
public:
  Rcpp::Function errorhandler;
  AMPLErrorHandler(Rcpp::Function _errorhandler): errorhandler(_errorhandler) { }
  void error(const ampl::AMPLException& e) {
    errorhandler(
      Rcpp::List::create(
        Rcpp::Named("type") = "error",
        Rcpp::Named("filename") = e.getSourceName(),
        Rcpp::Named("line") = e.getLineNumber(),
        Rcpp::Named("offset") = e.getOffset(),
        Rcpp::Named("message") = e.getMessage()
      )
    );
  }
  void warning(const ampl::AMPLException& e) {
    errorhandler(
      Rcpp::List::create(
        Rcpp::Named("type") = "warning",
        Rcpp::Named("filename") = e.getSourceName(),
        Rcpp::Named("line") = e.getLineNumber(),
        Rcpp::Named("offset") = e.getOffset(),
        Rcpp::Named("message") = e.getMessage()
      )
    );
  }
};

class RAMPL{
private:
  AMPLOutputHandler *OHandler = NULL;
  AMPLErrorHandler *EHandler = NULL;
public:
  ampl::AMPL _impl;
  RAMPL();
  RAMPL(SEXP);

  Rcpp::String cd();
  Rcpp::String cdStr(std::string);

  void setOption(std::string name, std::string value);
  Rcpp::String getOption(std::string);

  void read(std::string fileName);
  void readData(std::string fileName);

  void eval(std::string amplstatements);
  void solve();

  RVariableEntity getVariable(std::string name) const;
  RConstraintEntity getConstraint(std::string name) const;
  RObjectiveEntity getObjective(std::string name) const;
  RSetEntity getSet(std::string name) const;
  RParameterEntity getParameter(std::string name) const;

  Rcpp::List getVariables() const;
  Rcpp::List getConstraints() const;
  Rcpp::List getObjectives() const;
  Rcpp::List getSets() const;
  Rcpp::List getParameters() const;

  void setOutputHandler(Rcpp::Function outputhandler);
  Rcpp::Function getOutputHandler() const;
  void setErrorHandler(Rcpp::Function errorhandler);
  Rcpp::Function getErrorHandler() const;
};

RCPP_EXPOSED_ENUM_NODECL(ampl::output::Kind);

#endif