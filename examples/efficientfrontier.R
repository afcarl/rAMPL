efficientfrontier <- function(solver=NULL, modelDirectory=NULL) {
  library(rAMPL)
  ampl <- new(AMPL)

  if (!is.null(solver)) {
    ampl$setOption("solver", solver)
  } else {
    # ampl$setOption("solver", "cplex")
  }

  # Read the model file.
  if (is.null(modelDirectory)) {
    modelDirectory <- "./models"
  }
  modelDirectory <- paste(modelDirectory, "qpmv", sep="/")

  # Number of steps of the efficient frontier
  steps = 10

  ampl$setOption("reset_initial_guesses", TRUE)
  ampl$setOption("send_statuses", FALSE)

  # Load the AMPL model from file
  ampl$read(paste(modelDirectory, "qpmv.mod", sep="/"))
  ampl$read(paste(modelDirectory, "qpmvbit.run", sep="/"))

  # Set tables directory (parameter used in the script above)
  ampl$getParameter("data_dir")$set(modelDirectory)
  # Read tables
  ampl$readTable("assetstable")
  ampl$readTable("astrets")

  portfolioReturn <- ampl$getVariable("portret")
  averageReturn <- ampl$getParameter("averret")
  targetReturn <- ampl$getParameter("targetret")
  variance <- ampl$getObjective("cst")

  # Relax the integrality
  ampl$setOption("relax_integrality", TRUE)
  # Solve the problem
  ampl$solve()
  # Calibrate the efficient frontier range
  minret <- portfolioReturn$value()
  values <- averageReturn$getValues()
  col <- values[, "averret"]
  maxret <- max(col)
  stepsize <- (maxret - minret) / steps

  returns <- c()
  variances <- c()
  for (i in 0:(steps-1)) {
    cat(sprintf("Solving for return = %g\n", maxret - (i - 1) * stepsize))
    # Set target return to the desired point
    targetReturn$set(maxret - (i - 1) * stepsize)
    ampl$eval("let stockopall:={};let stockrun:=stockall;")
    # Relax integrality
    ampl$setOption("relax_integrality", TRUE)
    ampl$solve()
    cat(sprintf("QP result = %g\n", variance$value()))
    # Adjust included stocks
    ampl$eval("let stockrun:={i in stockrun:weights[i]>0};")
    ampl$eval("let stockopall:={i in stockrun:weights[i]>0.5};")
    # Set integrality back
    ampl$setOption("relax_integrality", FALSE)
    ampl$solve()
    cat(sprintf("QMIP result = %g\n", variance$value()))
    # Store data of corrent frontier point
    returns <- c(returns, maxret - (i - 1) * stepsize)
    variances <- c(variances, variance$value())
  }

  # Display efficient frontier points
  cat(sprintf("RETURN    VARIANCE\n"))
  for (i in 1:steps) {
    cat(sprintf("%-6f  %-6f\n", returns[i], variances[i]))
  }
}
