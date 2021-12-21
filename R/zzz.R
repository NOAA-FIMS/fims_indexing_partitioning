# Load RASAMS module -------------------------------------------------------------------------
.onLoad <- function(libname, pkgname) {
  print("loading FIP")
  library.dynam("FIP", pkgname, libname)
  
}

.LastLib <- function(libpath)
{
  print("unloading FIP")
  library.dynam.unload("FIP", libpath)
}
