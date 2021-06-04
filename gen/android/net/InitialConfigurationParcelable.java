/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
public class InitialConfigurationParcelable implements android.os.Parcelable
{

  public android.net.LinkAddress[] ipAddresses;

  public android.net.IpPrefix[] directlyConnectedRoutes;

  public java.lang.String[] dnsServers;

  public java.lang.String gateway;
  public static final android.os.Parcelable.Creator<InitialConfigurationParcelable> CREATOR = new android.os.Parcelable.Creator<InitialConfigurationParcelable>() {
    @Override
    public InitialConfigurationParcelable createFromParcel(android.os.Parcel _aidl_source) {
      InitialConfigurationParcelable _aidl_out = new InitialConfigurationParcelable();
      _aidl_out.readFromParcel(_aidl_source);
      return _aidl_out;
    }
    @Override
    public InitialConfigurationParcelable[] newArray(int _aidl_size) {
      return new InitialConfigurationParcelable[_aidl_size];
    }
  };
  @Override public final void writeToParcel(android.os.Parcel _aidl_parcel, int _aidl_flag)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.writeInt(0);
    _aidl_parcel.writeTypedArray(ipAddresses, 0);
    _aidl_parcel.writeTypedArray(directlyConnectedRoutes, 0);
    _aidl_parcel.writeStringArray(dnsServers);
    _aidl_parcel.writeString(gateway);
    int _aidl_end_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.setDataPosition(_aidl_start_pos);
    _aidl_parcel.writeInt(_aidl_end_pos - _aidl_start_pos);
    _aidl_parcel.setDataPosition(_aidl_end_pos);
  }
  public final void readFromParcel(android.os.Parcel _aidl_parcel)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    int _aidl_parcelable_size = _aidl_parcel.readInt();
    if (_aidl_parcelable_size < 0) return;
    try {
      ipAddresses = _aidl_parcel.createTypedArray(android.net.LinkAddress.CREATOR);
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      directlyConnectedRoutes = _aidl_parcel.createTypedArray(android.net.IpPrefix.CREATOR);
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      dnsServers = _aidl_parcel.createStringArray();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      gateway = _aidl_parcel.readString();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
    } finally {
      _aidl_parcel.setDataPosition(_aidl_start_pos + _aidl_parcelable_size);
    }
  }
  @Override public int describeContents()
  {
    return 0;
  }
}
