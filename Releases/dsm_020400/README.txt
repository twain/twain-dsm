This is release 2.4 of the TWAIN Data Source Manager.

Releases accompany updates to the TWAIN Specification, even when there are no
actual code changes in the DSM.  We do this to reduce confusion even though all
DSMs must interoperate with any application or data source, regardless of their
supported TWAIN Protocol.

Release Notes:

- We have finally pulled the trigger on the Linux TW_INT32/TW_UINT32 definition
  problem. TWAIN DSM 2.3.2 is the last version that has them defined a "long",
  which made them 64-bit on 64-bit OS's.  Starting with TWAIN 2.4 they are "int"
  and therefore 32-bit.  Linux applications and drivers that use the 2.3 TWAIN.H
  or earlier must use the 2.3.2 DSM.  All others must use a DSM that is 2.4 or
  later.

- The TWAIN Working Group is releasing its open source DSM on macOS for the
  first time.  Applications and data sources must switch to this version, since
  Apple no longer permits changes to the version that comes with the OS.

  Applications will find this new DSM in /Library/Frameworks/TWAINDSM.framwork.
  It is plug compatible with the older DSM, but requires some minor changes in
  the application, such as calling DG_CONTROL / DAT_ENTRYPOINT / MSG_GET,
  specifying DF_APP2 in the application's TW_IDENTITY.SupportedGroups, and
  providing a destination (not NULL) to the DG_CONTROL / DAT_CALLBACK /
  MSG_REGISTER_CALLBACK opertation.

  Data Sources must process DG_CONTROL / DAT_ENTRYPOINT / MSG_SET, they must
  report DF_DS2 in TW_IDENTITY.SupportedGroups, and they must use DG_CONTROL /
  DAT_NULL / MSG_* operation, instead of the MSG_INVOKE_CALLBACK, when sending
  events back to the application.

