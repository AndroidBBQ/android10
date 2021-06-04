/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.accounts;
/**
 * Central application service that provides account management.
 * @hide
 */
public interface IAccountManager extends android.os.IInterface
{
  /** Default implementation for IAccountManager. */
  public static class Default implements android.accounts.IAccountManager
  {
    @Override public java.lang.String getPassword(android.accounts.Account account) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getUserData(android.accounts.Account account, java.lang.String key) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.accounts.AuthenticatorDescription[] getAuthenticatorTypes(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.accounts.Account[] getAccounts(java.lang.String accountType, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.accounts.Account[] getAccountsForPackage(java.lang.String packageName, int uid, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.accounts.Account[] getAccountsByTypeForPackage(java.lang.String type, java.lang.String packageName, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.accounts.Account[] getAccountsAsUser(java.lang.String accountType, int userId, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void hasFeatures(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void getAccountByTypeAndFeatures(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void getAccountsByFeatures(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public boolean addAccountExplicitly(android.accounts.Account account, java.lang.String password, android.os.Bundle extras) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void removeAccount(android.accounts.IAccountManagerResponse response, android.accounts.Account account, boolean expectActivityLaunch) throws android.os.RemoteException
    {
    }
    @Override public void removeAccountAsUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, boolean expectActivityLaunch, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean removeAccountExplicitly(android.accounts.Account account) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void copyAccountToUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, int userFrom, int userTo) throws android.os.RemoteException
    {
    }
    @Override public void invalidateAuthToken(java.lang.String accountType, java.lang.String authToken) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String peekAuthToken(android.accounts.Account account, java.lang.String authTokenType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setAuthToken(android.accounts.Account account, java.lang.String authTokenType, java.lang.String authToken) throws android.os.RemoteException
    {
    }
    @Override public void setPassword(android.accounts.Account account, java.lang.String password) throws android.os.RemoteException
    {
    }
    @Override public void clearPassword(android.accounts.Account account) throws android.os.RemoteException
    {
    }
    @Override public void setUserData(android.accounts.Account account, java.lang.String key, java.lang.String value) throws android.os.RemoteException
    {
    }
    @Override public void updateAppPermission(android.accounts.Account account, java.lang.String authTokenType, int uid, boolean value) throws android.os.RemoteException
    {
    }
    @Override public void getAuthToken(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean notifyOnAuthFailure, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public void addAccount(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public void addAccountAsUser(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
    }
    @Override public void updateCredentials(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public void editProperties(android.accounts.IAccountManagerResponse response, java.lang.String accountType, boolean expectActivityLaunch) throws android.os.RemoteException
    {
    }
    @Override public void confirmCredentialsAsUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, android.os.Bundle options, boolean expectActivityLaunch, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean accountAuthenticated(android.accounts.Account account) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void getAuthTokenLabel(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType) throws android.os.RemoteException
    {
    }
    /* Shared accounts */
    @Override public void addSharedAccountsFromParentUser(int parentUserId, int userId, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    /* Account renaming. */
    @Override public void renameAccount(android.accounts.IAccountManagerResponse response, android.accounts.Account accountToRename, java.lang.String newName) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getPreviousName(android.accounts.Account account) throws android.os.RemoteException
    {
      return null;
    }
    /* Add account in two steps. */
    @Override public void startAddAccountSession(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    /* Update credentials in two steps. */
    @Override public void startUpdateCredentialsSession(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    /* Finish session started by startAddAccountSession(...) or startUpdateCredentialsSession(...)
        for user */
    @Override public void finishSessionAsUser(android.accounts.IAccountManagerResponse response, android.os.Bundle sessionBundle, boolean expectActivityLaunch, android.os.Bundle appInfo, int userId) throws android.os.RemoteException
    {
    }
    /* Check if an account exists on any user on the device. */
    @Override public boolean someUserHasAccount(android.accounts.Account account) throws android.os.RemoteException
    {
      return false;
    }
    /* Check if credentials update is suggested */
    @Override public void isCredentialsUpdateSuggested(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String statusToken) throws android.os.RemoteException
    {
    }
    /* Returns Map<String, Integer> from package name to visibility with all values stored for given account */
    @Override public java.util.Map getPackagesAndVisibilityForAccount(android.accounts.Account account) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean addAccountExplicitlyWithVisibility(android.accounts.Account account, java.lang.String password, android.os.Bundle extras, java.util.Map visibility) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setAccountVisibility(android.accounts.Account a, java.lang.String packageName, int newVisibility) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getAccountVisibility(android.accounts.Account a, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    /* Type may be null returns Map <Account, Integer>*/
    @Override public java.util.Map getAccountsAndVisibilityForPackage(java.lang.String packageName, java.lang.String accountType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void registerAccountListener(java.lang.String[] accountTypes, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void unregisterAccountListener(java.lang.String[] accountTypes, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    /* Check if the package in a user can access an account */
    @Override public boolean hasAccountAccess(android.accounts.Account account, java.lang.String packageName, android.os.UserHandle userHandle) throws android.os.RemoteException
    {
      return false;
    }
    /* Crate an intent to request account access for package and a given user id */
    @Override public android.content.IntentSender createRequestAccountAccessIntentSenderAsUser(android.accounts.Account account, java.lang.String packageName, android.os.UserHandle userHandle) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void onAccountAccessed(java.lang.String token) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.accounts.IAccountManager
  {
    private static final java.lang.String DESCRIPTOR = "android.accounts.IAccountManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.accounts.IAccountManager interface,
     * generating a proxy if needed.
     */
    public static android.accounts.IAccountManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.accounts.IAccountManager))) {
        return ((android.accounts.IAccountManager)iin);
      }
      return new android.accounts.IAccountManager.Stub.Proxy(obj);
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
        case TRANSACTION_getPassword:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _result = this.getPassword(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getUserData:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getUserData(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getAuthenticatorTypes:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.accounts.AuthenticatorDescription[] _result = this.getAuthenticatorTypes(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getAccounts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.accounts.Account[] _result = this.getAccounts(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getAccountsForPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.accounts.Account[] _result = this.getAccountsForPackage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getAccountsByTypeForPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.accounts.Account[] _result = this.getAccountsByTypeForPackage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getAccountsAsUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.accounts.Account[] _result = this.getAccountsAsUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_hasFeatures:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.hasFeatures(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAccountByTypeAndFeatures:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.getAccountByTypeAndFeatures(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAccountsByFeatures:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.getAccountsByFeatures(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addAccountExplicitly:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.addAccountExplicitly(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeAccount:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.removeAccount(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeAccountAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          this.removeAccountAsUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeAccountExplicitly:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.removeAccountExplicitly(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_copyAccountToUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.copyAccountToUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_invalidateAuthToken:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.invalidateAuthToken(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_peekAuthToken:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.peekAuthToken(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setAuthToken:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setAuthToken(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPassword:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setPassword(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearPassword:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.clearPassword(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUserData:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setUserData(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateAppPermission:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.updateAppPermission(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAuthToken:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.getAuthToken(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addAccount:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.addAccount(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addAccountAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          int _arg6;
          _arg6 = data.readInt();
          this.addAccountAsUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateCredentials:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.updateCredentials(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_editProperties:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.editProperties(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_confirmCredentialsAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          int _arg4;
          _arg4 = data.readInt();
          this.confirmCredentialsAsUser(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_accountAuthenticated:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.accountAuthenticated(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAuthTokenLabel:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.getAuthTokenLabel(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addSharedAccountsFromParentUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.addSharedAccountsFromParentUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_renameAccount:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.renameAccount(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPreviousName:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _result = this.getPreviousName(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_startAddAccountSession:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.startAddAccountSession(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startUpdateCredentialsSession:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.startUpdateCredentialsSession(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_finishSessionAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          this.finishSessionAsUser(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_someUserHasAccount:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.someUserHasAccount(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isCredentialsUpdateSuggested:
        {
          data.enforceInterface(descriptor);
          android.accounts.IAccountManagerResponse _arg0;
          _arg0 = android.accounts.IAccountManagerResponse.Stub.asInterface(data.readStrongBinder());
          android.accounts.Account _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.isCredentialsUpdateSuggested(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPackagesAndVisibilityForAccount:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.util.Map _result = this.getPackagesAndVisibilityForAccount(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_addAccountExplicitlyWithVisibility:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.util.Map _arg3;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg3 = data.readHashMap(cl);
          boolean _result = this.addAccountExplicitlyWithVisibility(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAccountVisibility:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.setAccountVisibility(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAccountVisibility:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getAccountVisibility(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAccountsAndVisibilityForPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.Map _result = this.getAccountsAndVisibilityForPackage(_arg0, _arg1);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_registerAccountListener:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _arg0;
          _arg0 = data.createStringArray();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.registerAccountListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterAccountListener:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _arg0;
          _arg0 = data.createStringArray();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.unregisterAccountListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasAccountAccess:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.hasAccountAccess(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_createRequestAccountAccessIntentSenderAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.IntentSender _result = this.createRequestAccountAccessIntentSenderAsUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_onAccountAccessed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onAccountAccessed(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.accounts.IAccountManager
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
      @Override public java.lang.String getPassword(android.accounts.Account account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPassword, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPassword(account);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String getUserData(android.accounts.Account account, java.lang.String key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUserData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUserData(account, key);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.accounts.AuthenticatorDescription[] getAuthenticatorTypes(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.accounts.AuthenticatorDescription[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAuthenticatorTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAuthenticatorTypes(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.accounts.AuthenticatorDescription.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.accounts.Account[] getAccounts(java.lang.String accountType, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.accounts.Account[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(accountType);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccounts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccounts(accountType, opPackageName);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.accounts.Account.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.accounts.Account[] getAccountsForPackage(java.lang.String packageName, int uid, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.accounts.Account[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(uid);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccountsForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccountsForPackage(packageName, uid, opPackageName);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.accounts.Account.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.accounts.Account[] getAccountsByTypeForPackage(java.lang.String type, java.lang.String packageName, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.accounts.Account[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(type);
          _data.writeString(packageName);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccountsByTypeForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccountsByTypeForPackage(type, packageName, opPackageName);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.accounts.Account.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.accounts.Account[] getAccountsAsUser(java.lang.String accountType, int userId, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.accounts.Account[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(accountType);
          _data.writeInt(userId);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccountsAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccountsAsUser(accountType, userId, opPackageName);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.accounts.Account.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void hasFeatures(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(features);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasFeatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hasFeatures(response, account, features, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void getAccountByTypeAndFeatures(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          _data.writeString(accountType);
          _data.writeStringArray(features);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccountByTypeAndFeatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAccountByTypeAndFeatures(response, accountType, features, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void getAccountsByFeatures(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          _data.writeString(accountType);
          _data.writeStringArray(features);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccountsByFeatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAccountsByFeatures(response, accountType, features, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean addAccountExplicitly(android.accounts.Account account, java.lang.String password, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(password);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAccountExplicitly, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addAccountExplicitly(account, password, extras);
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
      @Override public void removeAccount(android.accounts.IAccountManagerResponse response, android.accounts.Account account, boolean expectActivityLaunch) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAccount(response, account, expectActivityLaunch);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeAccountAsUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, boolean expectActivityLaunch, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAccountAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAccountAsUser(response, account, expectActivityLaunch, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean removeAccountExplicitly(android.accounts.Account account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAccountExplicitly, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeAccountExplicitly(account);
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
      @Override public void copyAccountToUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, int userFrom, int userTo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userFrom);
          _data.writeInt(userTo);
          boolean _status = mRemote.transact(Stub.TRANSACTION_copyAccountToUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().copyAccountToUser(response, account, userFrom, userTo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void invalidateAuthToken(java.lang.String accountType, java.lang.String authToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(accountType);
          _data.writeString(authToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_invalidateAuthToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().invalidateAuthToken(accountType, authToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String peekAuthToken(android.accounts.Account account, java.lang.String authTokenType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authTokenType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_peekAuthToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().peekAuthToken(account, authTokenType);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setAuthToken(android.accounts.Account account, java.lang.String authTokenType, java.lang.String authToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authTokenType);
          _data.writeString(authToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAuthToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAuthToken(account, authTokenType, authToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPassword(android.accounts.Account account, java.lang.String password) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(password);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPassword, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPassword(account, password);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearPassword(android.accounts.Account account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearPassword, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearPassword(account);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setUserData(android.accounts.Account account, java.lang.String key, java.lang.String value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(key);
          _data.writeString(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserData(account, key, value);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateAppPermission(android.accounts.Account account, java.lang.String authTokenType, int uid, boolean value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authTokenType);
          _data.writeInt(uid);
          _data.writeInt(((value)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAppPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateAppPermission(account, authTokenType, uid, value);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void getAuthToken(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean notifyOnAuthFailure, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authTokenType);
          _data.writeInt(((notifyOnAuthFailure)?(1):(0)));
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAuthToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAuthToken(response, account, authTokenType, notifyOnAuthFailure, expectActivityLaunch, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addAccount(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          _data.writeString(accountType);
          _data.writeString(authTokenType);
          _data.writeStringArray(requiredFeatures);
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addAccount(response, accountType, authTokenType, requiredFeatures, expectActivityLaunch, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addAccountAsUser(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          _data.writeString(accountType);
          _data.writeString(authTokenType);
          _data.writeStringArray(requiredFeatures);
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAccountAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addAccountAsUser(response, accountType, authTokenType, requiredFeatures, expectActivityLaunch, options, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateCredentials(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authTokenType);
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCredentials, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateCredentials(response, account, authTokenType, expectActivityLaunch, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void editProperties(android.accounts.IAccountManagerResponse response, java.lang.String accountType, boolean expectActivityLaunch) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          _data.writeString(accountType);
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_editProperties, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().editProperties(response, accountType, expectActivityLaunch);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void confirmCredentialsAsUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, android.os.Bundle options, boolean expectActivityLaunch, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_confirmCredentialsAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().confirmCredentialsAsUser(response, account, options, expectActivityLaunch, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean accountAuthenticated(android.accounts.Account account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_accountAuthenticated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().accountAuthenticated(account);
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
      @Override public void getAuthTokenLabel(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          _data.writeString(accountType);
          _data.writeString(authTokenType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAuthTokenLabel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAuthTokenLabel(response, accountType, authTokenType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Shared accounts */
      @Override public void addSharedAccountsFromParentUser(int parentUserId, int userId, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(parentUserId);
          _data.writeInt(userId);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSharedAccountsFromParentUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSharedAccountsFromParentUser(parentUserId, userId, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Account renaming. */
      @Override public void renameAccount(android.accounts.IAccountManagerResponse response, android.accounts.Account accountToRename, java.lang.String newName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((accountToRename!=null)) {
            _data.writeInt(1);
            accountToRename.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(newName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_renameAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().renameAccount(response, accountToRename, newName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getPreviousName(android.accounts.Account account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPreviousName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPreviousName(account);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Add account in two steps. */
      @Override public void startAddAccountSession(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          _data.writeString(accountType);
          _data.writeString(authTokenType);
          _data.writeStringArray(requiredFeatures);
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startAddAccountSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startAddAccountSession(response, accountType, authTokenType, requiredFeatures, expectActivityLaunch, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Update credentials in two steps. */
      @Override public void startUpdateCredentialsSession(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authTokenType);
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startUpdateCredentialsSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startUpdateCredentialsSession(response, account, authTokenType, expectActivityLaunch, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Finish session started by startAddAccountSession(...) or startUpdateCredentialsSession(...)
          for user */
      @Override public void finishSessionAsUser(android.accounts.IAccountManagerResponse response, android.os.Bundle sessionBundle, boolean expectActivityLaunch, android.os.Bundle appInfo, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((sessionBundle!=null)) {
            _data.writeInt(1);
            sessionBundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((expectActivityLaunch)?(1):(0)));
          if ((appInfo!=null)) {
            _data.writeInt(1);
            appInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishSessionAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishSessionAsUser(response, sessionBundle, expectActivityLaunch, appInfo, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Check if an account exists on any user on the device. */
      @Override public boolean someUserHasAccount(android.accounts.Account account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_someUserHasAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().someUserHasAccount(account);
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
      /* Check if credentials update is suggested */
      @Override public void isCredentialsUpdateSuggested(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String statusToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((response!=null))?(response.asBinder()):(null)));
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(statusToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isCredentialsUpdateSuggested, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isCredentialsUpdateSuggested(response, account, statusToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Returns Map<String, Integer> from package name to visibility with all values stored for given account */
      @Override public java.util.Map getPackagesAndVisibilityForAccount(android.accounts.Account account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackagesAndVisibilityForAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackagesAndVisibilityForAccount(account);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean addAccountExplicitlyWithVisibility(android.accounts.Account account, java.lang.String password, android.os.Bundle extras, java.util.Map visibility) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(password);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeMap(visibility);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAccountExplicitlyWithVisibility, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addAccountExplicitlyWithVisibility(account, password, extras, visibility);
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
      @Override public boolean setAccountVisibility(android.accounts.Account a, java.lang.String packageName, int newVisibility) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((a!=null)) {
            _data.writeInt(1);
            a.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          _data.writeInt(newVisibility);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAccountVisibility, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAccountVisibility(a, packageName, newVisibility);
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
      @Override public int getAccountVisibility(android.accounts.Account a, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((a!=null)) {
            _data.writeInt(1);
            a.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccountVisibility, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccountVisibility(a, packageName);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Type may be null returns Map <Account, Integer>*/
      @Override public java.util.Map getAccountsAndVisibilityForPackage(java.lang.String packageName, java.lang.String accountType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(accountType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccountsAndVisibilityForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccountsAndVisibilityForPackage(packageName, accountType);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void registerAccountListener(java.lang.String[] accountTypes, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringArray(accountTypes);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerAccountListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerAccountListener(accountTypes, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterAccountListener(java.lang.String[] accountTypes, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringArray(accountTypes);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAccountListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAccountListener(accountTypes, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Check if the package in a user can access an account */
      @Override public boolean hasAccountAccess(android.accounts.Account account, java.lang.String packageName, android.os.UserHandle userHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          if ((userHandle!=null)) {
            _data.writeInt(1);
            userHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasAccountAccess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasAccountAccess(account, packageName, userHandle);
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
      /* Crate an intent to request account access for package and a given user id */
      @Override public android.content.IntentSender createRequestAccountAccessIntentSenderAsUser(android.accounts.Account account, java.lang.String packageName, android.os.UserHandle userHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.IntentSender _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          if ((userHandle!=null)) {
            _data.writeInt(1);
            userHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createRequestAccountAccessIntentSenderAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createRequestAccountAccessIntentSenderAsUser(account, packageName, userHandle);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.IntentSender.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void onAccountAccessed(java.lang.String token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAccountAccessed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAccountAccessed(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.accounts.IAccountManager sDefaultImpl;
    }
    static final int TRANSACTION_getPassword = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getUserData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getAuthenticatorTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getAccounts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getAccountsForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getAccountsByTypeForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getAccountsAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_hasFeatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getAccountByTypeAndFeatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getAccountsByFeatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_addAccountExplicitly = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_removeAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_removeAccountAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_removeAccountExplicitly = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_copyAccountToUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_invalidateAuthToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_peekAuthToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setAuthToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setPassword = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_clearPassword = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setUserData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_updateAppPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getAuthToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_addAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_addAccountAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_updateCredentials = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_editProperties = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_confirmCredentialsAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_accountAuthenticated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_getAuthTokenLabel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_addSharedAccountsFromParentUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_renameAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_getPreviousName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_startAddAccountSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_startUpdateCredentialsSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_finishSessionAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_someUserHasAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_isCredentialsUpdateSuggested = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_getPackagesAndVisibilityForAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_addAccountExplicitlyWithVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_setAccountVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getAccountVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_getAccountsAndVisibilityForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_registerAccountListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_unregisterAccountListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_hasAccountAccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_createRequestAccountAccessIntentSenderAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_onAccountAccessed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    public static boolean setDefaultImpl(android.accounts.IAccountManager impl) {
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
    public static android.accounts.IAccountManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.lang.String getPassword(android.accounts.Account account) throws android.os.RemoteException;
  public java.lang.String getUserData(android.accounts.Account account, java.lang.String key) throws android.os.RemoteException;
  public android.accounts.AuthenticatorDescription[] getAuthenticatorTypes(int userId) throws android.os.RemoteException;
  public android.accounts.Account[] getAccounts(java.lang.String accountType, java.lang.String opPackageName) throws android.os.RemoteException;
  public android.accounts.Account[] getAccountsForPackage(java.lang.String packageName, int uid, java.lang.String opPackageName) throws android.os.RemoteException;
  public android.accounts.Account[] getAccountsByTypeForPackage(java.lang.String type, java.lang.String packageName, java.lang.String opPackageName) throws android.os.RemoteException;
  public android.accounts.Account[] getAccountsAsUser(java.lang.String accountType, int userId, java.lang.String opPackageName) throws android.os.RemoteException;
  public void hasFeatures(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException;
  public void getAccountByTypeAndFeatures(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException;
  public void getAccountsByFeatures(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String[] features, java.lang.String opPackageName) throws android.os.RemoteException;
  public boolean addAccountExplicitly(android.accounts.Account account, java.lang.String password, android.os.Bundle extras) throws android.os.RemoteException;
  public void removeAccount(android.accounts.IAccountManagerResponse response, android.accounts.Account account, boolean expectActivityLaunch) throws android.os.RemoteException;
  public void removeAccountAsUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, boolean expectActivityLaunch, int userId) throws android.os.RemoteException;
  public boolean removeAccountExplicitly(android.accounts.Account account) throws android.os.RemoteException;
  public void copyAccountToUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, int userFrom, int userTo) throws android.os.RemoteException;
  public void invalidateAuthToken(java.lang.String accountType, java.lang.String authToken) throws android.os.RemoteException;
  public java.lang.String peekAuthToken(android.accounts.Account account, java.lang.String authTokenType) throws android.os.RemoteException;
  public void setAuthToken(android.accounts.Account account, java.lang.String authTokenType, java.lang.String authToken) throws android.os.RemoteException;
  public void setPassword(android.accounts.Account account, java.lang.String password) throws android.os.RemoteException;
  public void clearPassword(android.accounts.Account account) throws android.os.RemoteException;
  public void setUserData(android.accounts.Account account, java.lang.String key, java.lang.String value) throws android.os.RemoteException;
  public void updateAppPermission(android.accounts.Account account, java.lang.String authTokenType, int uid, boolean value) throws android.os.RemoteException;
  public void getAuthToken(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean notifyOnAuthFailure, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException;
  public void addAccount(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException;
  public void addAccountAsUser(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public void updateCredentials(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException;
  public void editProperties(android.accounts.IAccountManagerResponse response, java.lang.String accountType, boolean expectActivityLaunch) throws android.os.RemoteException;
  public void confirmCredentialsAsUser(android.accounts.IAccountManagerResponse response, android.accounts.Account account, android.os.Bundle options, boolean expectActivityLaunch, int userId) throws android.os.RemoteException;
  public boolean accountAuthenticated(android.accounts.Account account) throws android.os.RemoteException;
  public void getAuthTokenLabel(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType) throws android.os.RemoteException;
  /* Shared accounts */
  public void addSharedAccountsFromParentUser(int parentUserId, int userId, java.lang.String opPackageName) throws android.os.RemoteException;
  /* Account renaming. */
  public void renameAccount(android.accounts.IAccountManagerResponse response, android.accounts.Account accountToRename, java.lang.String newName) throws android.os.RemoteException;
  public java.lang.String getPreviousName(android.accounts.Account account) throws android.os.RemoteException;
  /* Add account in two steps. */
  public void startAddAccountSession(android.accounts.IAccountManagerResponse response, java.lang.String accountType, java.lang.String authTokenType, java.lang.String[] requiredFeatures, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException;
  /* Update credentials in two steps. */
  public void startUpdateCredentialsSession(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String authTokenType, boolean expectActivityLaunch, android.os.Bundle options) throws android.os.RemoteException;
  /* Finish session started by startAddAccountSession(...) or startUpdateCredentialsSession(...)
      for user */
  public void finishSessionAsUser(android.accounts.IAccountManagerResponse response, android.os.Bundle sessionBundle, boolean expectActivityLaunch, android.os.Bundle appInfo, int userId) throws android.os.RemoteException;
  /* Check if an account exists on any user on the device. */
  public boolean someUserHasAccount(android.accounts.Account account) throws android.os.RemoteException;
  /* Check if credentials update is suggested */
  public void isCredentialsUpdateSuggested(android.accounts.IAccountManagerResponse response, android.accounts.Account account, java.lang.String statusToken) throws android.os.RemoteException;
  /* Returns Map<String, Integer> from package name to visibility with all values stored for given account */
  public java.util.Map getPackagesAndVisibilityForAccount(android.accounts.Account account) throws android.os.RemoteException;
  public boolean addAccountExplicitlyWithVisibility(android.accounts.Account account, java.lang.String password, android.os.Bundle extras, java.util.Map visibility) throws android.os.RemoteException;
  public boolean setAccountVisibility(android.accounts.Account a, java.lang.String packageName, int newVisibility) throws android.os.RemoteException;
  public int getAccountVisibility(android.accounts.Account a, java.lang.String packageName) throws android.os.RemoteException;
  /* Type may be null returns Map <Account, Integer>*/
  public java.util.Map getAccountsAndVisibilityForPackage(java.lang.String packageName, java.lang.String accountType) throws android.os.RemoteException;
  public void registerAccountListener(java.lang.String[] accountTypes, java.lang.String opPackageName) throws android.os.RemoteException;
  public void unregisterAccountListener(java.lang.String[] accountTypes, java.lang.String opPackageName) throws android.os.RemoteException;
  /* Check if the package in a user can access an account */
  public boolean hasAccountAccess(android.accounts.Account account, java.lang.String packageName, android.os.UserHandle userHandle) throws android.os.RemoteException;
  /* Crate an intent to request account access for package and a given user id */
  public android.content.IntentSender createRequestAccountAccessIntentSenderAsUser(android.accounts.Account account, java.lang.String packageName, android.os.UserHandle userHandle) throws android.os.RemoteException;
  public void onAccountAccessed(java.lang.String token) throws android.os.RemoteException;
}
