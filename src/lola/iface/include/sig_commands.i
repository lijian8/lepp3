//FIXME: this is broken!!
//swig interface file for generating (python) wrappers
 
 %module control_commands	  //the module to be wrapped
 %header %{
 #include "control_commands.hpp"
 %}
 
 // Parse the original header file
 %include "control_commands.hpp"

