This is release 2.3.2 of the TWAIN Data Source Manager.

Releases accompany updates to the TWAIN Specification, even when there are no
actual code changes in the DSM.  We do this to reduce confusion even though all
DSMs must interoperate with any application or data source, regardless of their
supported TWAIN Protocol.

Release Notes:

- TWAIN DSM 2.3.2 is the last version that has them defined a "long", which
  made them 64-bit on 64-bit OS's.  Starting with TWAIN 2.4 they are "int"
  and therefore 32-bit.  Linux applications and drivers that use the 2.3 TWAIN.H
  or earlier must use the 2.3.2 DSM.  All others must use a DSM that is 2.4 or
  later.  This code will correctly filter out 2.4 drivers without crashing.

  There is no corresponding version for Windows or Mac, please use the latest
  available version of the DSM.
