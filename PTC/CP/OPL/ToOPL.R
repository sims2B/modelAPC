#!/usr/bin/env Rscript

ToOPL <- function(filename, outfile = NA, cdir = FALSE) {
  ToVector <- function(x) paste("[", paste(x, collapse = ", "), "]")
  ToMatrix <- function(x) paste("[\n", paste(apply(x, 1, ToVector), collapse = ",\n"), "\n]",  sep = "")

  x <- scan(filename, quiet = TRUE)
  nbJ <- x[1]
  nbM = x[2]
  nbF = x[3]
  offset <- seq(3 + nbJ + 1, length(x), 3 + nbM) 
  
  setups <- matrix(x[ offset + 2], nrow = nbF, ncol = nbF, byrow = TRUE)
  diag(setups) <- 0
  qualifs <- matrix(x[ rowSums(expand.grid(offset + 2, seq(nbM))) ], nrow = nbF, ncol = nbM)

  if(is.na(outfile)) {
    outfile <- ifelse(cdir, basename(filename), filename)
    outfile <- sprintf("%s-opl.dat", tools::file_path_sans_ext(outfile))
  }
  ## cat(filename)
  ## cat('\n')
  ## cat(outfile)
  fileConn<-file(outfile)
  writeLines( c(
    sprintf("nbM=%d;", nbM),
    sprintf("nbF=%d;", nbF),
    sprintf("fsizes=%s;", ToVector(as.vector(table(x[3 + seq(nbJ)])))),
    sprintf("durations=%s;", ToVector(x[ offset ])),
    sprintf("thresholds=%s;", ToVector(x[ offset + 1])),
    sprintf("setups=%s;", ToMatrix(setups)),
    sprintf("qualifications=%s;", ToMatrix(qualifs))
  ), fileConn)
  close(fileConn)
}


args <- commandArgs(trailingOnly=TRUE)
invisible(sapply(args, ToOPL))

## shell
## find datasets/ -name '*.txt' -exec Rscript ToOPL.R {} \;
