/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
public class DhcpResultsParcelable implements android.os.Parcelable
{

  public android.net.StaticIpConfiguration baseConfiguration;

  public int leaseDuration;

  public int mtu;

  public java.lang.String serverAddress;

  public java.lang.String vendorInfo;

  public java.lang.String serverHostName;
  public static final android.os.Parcelable.Creator<DhcpResultsParcelable> CREATOR = new android.os.Parcelable.Creator<DhcpResultsParcelable>() {
    @Override
    public DhcpResultsParcelable createFromParcel(android.os.Parcel _aidl_source) {
      DhcpResultsParcelable _aidl_out = new DhcpResultsParcelable();
      _aidl_out.readFromParcel(_aidl_source);
      return _aidl_out;
    }
    @Override
    public DhcpResultsParcelable[] newArray(int _aidl_size) {
      return new DhcpResultsParcelable[_aidl_size];
    }
  };
  @Override public final void writeToParcel(android.os.Parcel _aidl_parcel, int _aidl_flag)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.writeInt(0);
    if ((baseConfiguration!=null)) {
      _aidl_parcel.writeInt(1);
      baseConfiguration.writeToParcel(_aidl_parcel, 0);
    }
    else {
      _aidl_parcel.writeInt(0);
    }
    _aidl_parcel.writeInt(leaseDuration);
    _aidl_parcel.writeInt(mtu);
    _aidl_parcel.writeString(serverAddress);
    _aidl_parcel.writeString(vendorInfo);
    _aidl_parcel.writeString(serverHostName);
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
      if ((0!=_aidl_parcel.readInt())) {
        baseConfiguration = android.net.StaticIpConfiguration.CREATOR.createFromParcel(_aidl_parcel);
      }
      else {
        baseConfiguration = null;
      }
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      leaseDuration = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      mtu = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      serverAddress = _aidl_parcel.readString();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      vendorInfo = _aidl_parcel.readString();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      serverHostName = _aidl_parcel.readString();
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
