/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
/** Interface for applications to access the ICC phone book.
 *
 * <p>The following code snippet demonstrates a static method to
 * retrieve the IIccPhoneBook interface from Android:</p>
 * <pre>private static IIccPhoneBook getSimPhoneBookInterface()
            throws DeadObjectException {
    IServiceManager sm = ServiceManagerNative.getDefault();
    IIccPhoneBook spb;
    spb = IIccPhoneBook.Stub.asInterface(sm.getService("iccphonebook"));
    return spb;
}
 * </pre>
 */
public interface IIccPhoneBook extends android.os.IInterface
{
  /** Default implementation for IIccPhoneBook. */
  public static class Default implements com.android.internal.telephony.IIccPhoneBook
  {
    /**
         * Loads the AdnRecords in efid and returns them as a
         * List of AdnRecords
         *
         * @param efid the EF id of a ADN-like SIM
         * @return List of AdnRecord
         */
    @Override public java.util.List<com.android.internal.telephony.uicc.AdnRecord> getAdnRecordsInEf(int efid) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Loads the AdnRecords in efid and returns them as a
         * List of AdnRecords
         *
         * @param efid the EF id of a ADN-like SIM
         * @param subId user preferred subId
         * @return List of AdnRecord
         */
    @Override public java.util.List<com.android.internal.telephony.uicc.AdnRecord> getAdnRecordsInEfForSubscriber(int subId, int efid) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Replace oldAdn with newAdn in ADN-like record in EF
         *
         * getAdnRecordsInEf must be called at least once before this function,
         * otherwise an error will be returned
         *
         * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
         * @param oldTag adn tag to be replaced
         * @param oldPhoneNumber adn number to be replaced
         *        Set both oldTag and oldPhoneNubmer to "" means to replace an
         *        empty record, aka, insert new record
         * @param newTag adn tag to be stored
         * @param newPhoneNumber adn number ot be stored
         *        Set both newTag and newPhoneNubmer to "" means to replace the old
         *        record with empty one, aka, delete old record
         * @param pin2 required to update EF_FDN, otherwise must be null
         * @return true for success
         */
    @Override public boolean updateAdnRecordsInEfBySearch(int efid, java.lang.String oldTag, java.lang.String oldPhoneNumber, java.lang.String newTag, java.lang.String newPhoneNumber, java.lang.String pin2) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Replace oldAdn with newAdn in ADN-like record in EF
         *
         * getAdnRecordsInEf must be called at least once before this function,
         * otherwise an error will be returned
         *
         * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
         * @param oldTag adn tag to be replaced
         * @param oldPhoneNumber adn number to be replaced
         *        Set both oldTag and oldPhoneNubmer to "" means to replace an
         *        empty record, aka, insert new record
         * @param newTag adn tag to be stored
         * @param newPhoneNumber adn number ot be stored
         *        Set both newTag and newPhoneNubmer to "" means to replace the old
         *        record with empty one, aka, delete old record
         * @param pin2 required to update EF_FDN, otherwise must be null
         * @param subId user preferred subId
         * @return true for success
         */
    @Override public boolean updateAdnRecordsInEfBySearchForSubscriber(int subId, int efid, java.lang.String oldTag, java.lang.String oldPhoneNumber, java.lang.String newTag, java.lang.String newPhoneNumber, java.lang.String pin2) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Update an ADN-like EF record by record index
         *
         * This is useful for iteration the whole ADN file, such as write the whole
         * phone book or erase/format the whole phonebook
         *
         * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
         * @param newTag adn tag to be stored
         * @param newPhoneNumber adn number to be stored
         *        Set both newTag and newPhoneNubmer to "" means to replace the old
         *        record with empty one, aka, delete old record
         * @param index is 1-based adn record index to be updated
         * @param pin2 required to update EF_FDN, otherwise must be null
         * @return true for success
         */
    @Override public boolean updateAdnRecordsInEfByIndex(int efid, java.lang.String newTag, java.lang.String newPhoneNumber, int index, java.lang.String pin2) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Update an ADN-like EF record by record index
         *
         * This is useful for iteration the whole ADN file, such as write the whole
         * phone book or erase/format the whole phonebook
         *
         * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
         * @param newTag adn tag to be stored
         * @param newPhoneNumber adn number to be stored
         *        Set both newTag and newPhoneNubmer to "" means to replace the old
         *        record with empty one, aka, delete old record
         * @param index is 1-based adn record index to be updated
         * @param pin2 required to update EF_FDN, otherwise must be null
         * @param subId user preferred subId
         * @return true for success
         */
    @Override public boolean updateAdnRecordsInEfByIndexForSubscriber(int subId, int efid, java.lang.String newTag, java.lang.String newPhoneNumber, int index, java.lang.String pin2) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get the max munber of records in efid
         *
         * @param efid the EF id of a ADN-like SIM
         * @return  int[3] array
         *            recordSizes[0]  is the single record length
         *            recordSizes[1]  is the total length of the EF file
         *            recordSizes[2]  is the number of records in the EF file
         */
    @Override public int[] getAdnRecordsSize(int efid) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the max munber of records in efid
         *
         * @param efid the EF id of a ADN-like SIM
         * @param subId user preferred subId
         * @return  int[3] array
         *            recordSizes[0]  is the single record length
         *            recordSizes[1]  is the total length of the EF file
         *            recordSizes[2]  is the number of records in the EF file
         */
    @Override public int[] getAdnRecordsSizeForSubscriber(int subId, int efid) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.IIccPhoneBook
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.IIccPhoneBook";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.IIccPhoneBook interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.IIccPhoneBook asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.IIccPhoneBook))) {
        return ((com.android.internal.telephony.IIccPhoneBook)iin);
      }
      return new com.android.internal.telephony.IIccPhoneBook.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_getAdnRecordsInEf:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<com.android.internal.telephony.uicc.AdnRecord> _result = this.getAdnRecordsInEf(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getAdnRecordsInEfForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<com.android.internal.telephony.uicc.AdnRecord> _result = this.getAdnRecordsInEfForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_updateAdnRecordsInEfBySearch:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          java.lang.String _arg5;
          _arg5 = data.readString();
          boolean _result = this.updateAdnRecordsInEfBySearch(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updateAdnRecordsInEfBySearchForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          java.lang.String _arg5;
          _arg5 = data.readString();
          java.lang.String _arg6;
          _arg6 = data.readString();
          boolean _result = this.updateAdnRecordsInEfBySearchForSubscriber(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updateAdnRecordsInEfByIndex:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          boolean _result = this.updateAdnRecordsInEfByIndex(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updateAdnRecordsInEfByIndexForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          java.lang.String _arg5;
          _arg5 = data.readString();
          boolean _result = this.updateAdnRecordsInEfByIndexForSubscriber(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAdnRecordsSize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _result = this.getAdnRecordsSize(_arg0);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getAdnRecordsSizeForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int[] _result = this.getAdnRecordsSizeForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.IIccPhoneBook
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      /**
           * Loads the AdnRecords in efid and returns them as a
           * List of AdnRecords
           *
           * @param efid the EF id of a ADN-like SIM
           * @return List of AdnRecord
           */
      @Override public java.util.List<com.android.internal.telephony.uicc.AdnRecord> getAdnRecordsInEf(int efid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<com.android.internal.telephony.uicc.AdnRecord> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(efid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAdnRecordsInEf, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAdnRecordsInEf(efid);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(com.android.internal.telephony.uicc.AdnRecord.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Loads the AdnRecords in efid and returns them as a
           * List of AdnRecords
           *
           * @param efid the EF id of a ADN-like SIM
           * @param subId user preferred subId
           * @return List of AdnRecord
           */
      @Override public java.util.List<com.android.internal.telephony.uicc.AdnRecord> getAdnRecordsInEfForSubscriber(int subId, int efid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<com.android.internal.telephony.uicc.AdnRecord> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(efid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAdnRecordsInEfForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAdnRecordsInEfForSubscriber(subId, efid);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(com.android.internal.telephony.uicc.AdnRecord.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Replace oldAdn with newAdn in ADN-like record in EF
           *
           * getAdnRecordsInEf must be called at least once before this function,
           * otherwise an error will be returned
           *
           * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
           * @param oldTag adn tag to be replaced
           * @param oldPhoneNumber adn number to be replaced
           *        Set both oldTag and oldPhoneNubmer to "" means to replace an
           *        empty record, aka, insert new record
           * @param newTag adn tag to be stored
           * @param newPhoneNumber adn number ot be stored
           *        Set both newTag and newPhoneNubmer to "" means to replace the old
           *        record with empty one, aka, delete old record
           * @param pin2 required to update EF_FDN, otherwise must be null
           * @return true for success
           */
      @Override public boolean updateAdnRecordsInEfBySearch(int efid, java.lang.String oldTag, java.lang.String oldPhoneNumber, java.lang.String newTag, java.lang.String newPhoneNumber, java.lang.String pin2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(efid);
          _data.writeString(oldTag);
          _data.writeString(oldPhoneNumber);
          _data.writeString(newTag);
          _data.writeString(newPhoneNumber);
          _data.writeString(pin2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAdnRecordsInEfBySearch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateAdnRecordsInEfBySearch(efid, oldTag, oldPhoneNumber, newTag, newPhoneNumber, pin2);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Replace oldAdn with newAdn in ADN-like record in EF
           *
           * getAdnRecordsInEf must be called at least once before this function,
           * otherwise an error will be returned
           *
           * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
           * @param oldTag adn tag to be replaced
           * @param oldPhoneNumber adn number to be replaced
           *        Set both oldTag and oldPhoneNubmer to "" means to replace an
           *        empty record, aka, insert new record
           * @param newTag adn tag to be stored
           * @param newPhoneNumber adn number ot be stored
           *        Set both newTag and newPhoneNubmer to "" means to replace the old
           *        record with empty one, aka, delete old record
           * @param pin2 required to update EF_FDN, otherwise must be null
           * @param subId user preferred subId
           * @return true for success
           */
      @Override public boolean updateAdnRecordsInEfBySearchForSubscriber(int subId, int efid, java.lang.String oldTag, java.lang.String oldPhoneNumber, java.lang.String newTag, java.lang.String newPhoneNumber, java.lang.String pin2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(efid);
          _data.writeString(oldTag);
          _data.writeString(oldPhoneNumber);
          _data.writeString(newTag);
          _data.writeString(newPhoneNumber);
          _data.writeString(pin2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAdnRecordsInEfBySearchForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateAdnRecordsInEfBySearchForSubscriber(subId, efid, oldTag, oldPhoneNumber, newTag, newPhoneNumber, pin2);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Update an ADN-like EF record by record index
           *
           * This is useful for iteration the whole ADN file, such as write the whole
           * phone book or erase/format the whole phonebook
           *
           * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
           * @param newTag adn tag to be stored
           * @param newPhoneNumber adn number to be stored
           *        Set both newTag and newPhoneNubmer to "" means to replace the old
           *        record with empty one, aka, delete old record
           * @param index is 1-based adn record index to be updated
           * @param pin2 required to update EF_FDN, otherwise must be null
           * @return true for success
           */
      @Override public boolean updateAdnRecordsInEfByIndex(int efid, java.lang.String newTag, java.lang.String newPhoneNumber, int index, java.lang.String pin2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(efid);
          _data.writeString(newTag);
          _data.writeString(newPhoneNumber);
          _data.writeInt(index);
          _data.writeString(pin2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAdnRecordsInEfByIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateAdnRecordsInEfByIndex(efid, newTag, newPhoneNumber, index, pin2);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Update an ADN-like EF record by record index
           *
           * This is useful for iteration the whole ADN file, such as write the whole
           * phone book or erase/format the whole phonebook
           *
           * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
           * @param newTag adn tag to be stored
           * @param newPhoneNumber adn number to be stored
           *        Set both newTag and newPhoneNubmer to "" means to replace the old
           *        record with empty one, aka, delete old record
           * @param index is 1-based adn record index to be updated
           * @param pin2 required to update EF_FDN, otherwise must be null
           * @param subId user preferred subId
           * @return true for success
           */
      @Override public boolean updateAdnRecordsInEfByIndexForSubscriber(int subId, int efid, java.lang.String newTag, java.lang.String newPhoneNumber, int index, java.lang.String pin2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(efid);
          _data.writeString(newTag);
          _data.writeString(newPhoneNumber);
          _data.writeInt(index);
          _data.writeString(pin2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAdnRecordsInEfByIndexForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateAdnRecordsInEfByIndexForSubscriber(subId, efid, newTag, newPhoneNumber, index, pin2);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the max munber of records in efid
           *
           * @param efid the EF id of a ADN-like SIM
           * @return  int[3] array
           *            recordSizes[0]  is the single record length
           *            recordSizes[1]  is the total length of the EF file
           *            recordSizes[2]  is the number of records in the EF file
           */
      @Override public int[] getAdnRecordsSize(int efid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(efid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAdnRecordsSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAdnRecordsSize(efid);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the max munber of records in efid
           *
           * @param efid the EF id of a ADN-like SIM
           * @param subId user preferred subId
           * @return  int[3] array
           *            recordSizes[0]  is the single record length
           *            recordSizes[1]  is the total length of the EF file
           *            recordSizes[2]  is the number of records in the EF file
           */
      @Override public int[] getAdnRecordsSizeForSubscriber(int subId, int efid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(efid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAdnRecordsSizeForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAdnRecordsSizeForSubscriber(subId, efid);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.internal.telephony.IIccPhoneBook sDefaultImpl;
    }
    static final int TRANSACTION_getAdnRecordsInEf = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getAdnRecordsInEfForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_updateAdnRecordsInEfBySearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_updateAdnRecordsInEfBySearchForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_updateAdnRecordsInEfByIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_updateAdnRecordsInEfByIndexForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getAdnRecordsSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getAdnRecordsSizeForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(com.android.internal.telephony.IIccPhoneBook impl) {
      // Only one user of this interface can use this function
      // at a time. This is a heuristic to detect if two different
      // users in the same process use this function.
      if (Stub.Proxy.sDefaultImpl != null) {
        throw new IllegalStateException("setDefaultImpl() called twice");
      }
      if (impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static com.android.internal.telephony.IIccPhoneBook getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Loads the AdnRecords in efid and returns them as a
       * List of AdnRecords
       *
       * @param efid the EF id of a ADN-like SIM
       * @return List of AdnRecord
       */
  public java.util.List<com.android.internal.telephony.uicc.AdnRecord> getAdnRecordsInEf(int efid) throws android.os.RemoteException;
  /**
       * Loads the AdnRecords in efid and returns them as a
       * List of AdnRecords
       *
       * @param efid the EF id of a ADN-like SIM
       * @param subId user preferred subId
       * @return List of AdnRecord
       */
  public java.util.List<com.android.internal.telephony.uicc.AdnRecord> getAdnRecordsInEfForSubscriber(int subId, int efid) throws android.os.RemoteException;
  /**
       * Replace oldAdn with newAdn in ADN-like record in EF
       *
       * getAdnRecordsInEf must be called at least once before this function,
       * otherwise an error will be returned
       *
       * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
       * @param oldTag adn tag to be replaced
       * @param oldPhoneNumber adn number to be replaced
       *        Set both oldTag and oldPhoneNubmer to "" means to replace an
       *        empty record, aka, insert new record
       * @param newTag adn tag to be stored
       * @param newPhoneNumber adn number ot be stored
       *        Set both newTag and newPhoneNubmer to "" means to replace the old
       *        record with empty one, aka, delete old record
       * @param pin2 required to update EF_FDN, otherwise must be null
       * @return true for success
       */
  public boolean updateAdnRecordsInEfBySearch(int efid, java.lang.String oldTag, java.lang.String oldPhoneNumber, java.lang.String newTag, java.lang.String newPhoneNumber, java.lang.String pin2) throws android.os.RemoteException;
  /**
       * Replace oldAdn with newAdn in ADN-like record in EF
       *
       * getAdnRecordsInEf must be called at least once before this function,
       * otherwise an error will be returned
       *
       * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
       * @param oldTag adn tag to be replaced
       * @param oldPhoneNumber adn number to be replaced
       *        Set both oldTag and oldPhoneNubmer to "" means to replace an
       *        empty record, aka, insert new record
       * @param newTag adn tag to be stored
       * @param newPhoneNumber adn number ot be stored
       *        Set both newTag and newPhoneNubmer to "" means to replace the old
       *        record with empty one, aka, delete old record
       * @param pin2 required to update EF_FDN, otherwise must be null
       * @param subId user preferred subId
       * @return true for success
       */
  public boolean updateAdnRecordsInEfBySearchForSubscriber(int subId, int efid, java.lang.String oldTag, java.lang.String oldPhoneNumber, java.lang.String newTag, java.lang.String newPhoneNumber, java.lang.String pin2) throws android.os.RemoteException;
  /**
       * Update an ADN-like EF record by record index
       *
       * This is useful for iteration the whole ADN file, such as write the whole
       * phone book or erase/format the whole phonebook
       *
       * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
       * @param newTag adn tag to be stored
       * @param newPhoneNumber adn number to be stored
       *        Set both newTag and newPhoneNubmer to "" means to replace the old
       *        record with empty one, aka, delete old record
       * @param index is 1-based adn record index to be updated
       * @param pin2 required to update EF_FDN, otherwise must be null
       * @return true for success
       */
  public boolean updateAdnRecordsInEfByIndex(int efid, java.lang.String newTag, java.lang.String newPhoneNumber, int index, java.lang.String pin2) throws android.os.RemoteException;
  /**
       * Update an ADN-like EF record by record index
       *
       * This is useful for iteration the whole ADN file, such as write the whole
       * phone book or erase/format the whole phonebook
       *
       * @param efid must be one among EF_ADN, EF_FDN, and EF_SDN
       * @param newTag adn tag to be stored
       * @param newPhoneNumber adn number to be stored
       *        Set both newTag and newPhoneNubmer to "" means to replace the old
       *        record with empty one, aka, delete old record
       * @param index is 1-based adn record index to be updated
       * @param pin2 required to update EF_FDN, otherwise must be null
       * @param subId user preferred subId
       * @return true for success
       */
  public boolean updateAdnRecordsInEfByIndexForSubscriber(int subId, int efid, java.lang.String newTag, java.lang.String newPhoneNumber, int index, java.lang.String pin2) throws android.os.RemoteException;
  /**
       * Get the max munber of records in efid
       *
       * @param efid the EF id of a ADN-like SIM
       * @return  int[3] array
       *            recordSizes[0]  is the single record length
       *            recordSizes[1]  is the total length of the EF file
       *            recordSizes[2]  is the number of records in the EF file
       */
  public int[] getAdnRecordsSize(int efid) throws android.os.RemoteException;
  /**
       * Get the max munber of records in efid
       *
       * @param efid the EF id of a ADN-like SIM
       * @param subId user preferred subId
       * @return  int[3] array
       *            recordSizes[0]  is the single record length
       *            recordSizes[1]  is the total length of the EF file
       *            recordSizes[2]  is the number of records in the EF file
       */
  public int[] getAdnRecordsSizeForSubscriber(int subId, int efid) throws android.os.RemoteException;
}
