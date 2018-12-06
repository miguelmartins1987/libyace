# libyace
**Y**et **A**nother **C**ommand **E**xecutor

Built for Linux.

The purpose of this library is to invoke external processes via PL/SQL. The provided `PkgYace.ExecuteCommand` routine invokes the desired external command and returns its exit code, its standard output, and its standard error (the latter two truncated to the maximum amount allowed by a `VARCHAR2` type).

You may or may not have to append the following line to your `extproc.ora` file in order to execute certain commands:

```
SET PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin
```
