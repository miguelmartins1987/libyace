/*
Copyright (c) 2018 Miguel Martins

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

create or replace package PkgYace is

  function ExecuteCommand(command varchar2,
                          output  out varchar2,
                          error   out varchar2) return PLS_INTEGER;

end PkgYace;
/
create or replace package body PkgYace is

  function ExecuteCommand(command varchar2,
                          output  out varchar2,
                          error   out varchar2) return PLS_INTEGER as
    LANGUAGE C NAME "ExecuteCommand"
    LIBRARY LIBYACE
    WITH CONTEXT
    PARAMETERS(CONTEXT,
               command STRING,
               command INDICATOR short,
               command LENGTH int,
               output  STRING,
               output  INDICATOR short,
               output  LENGTH int,
               error   STRING,
               error   INDICATOR short,
               error   LENGTH int,
               RETURN  INDICATOR,
               RETURN  INT);

end PkgYace;
/
