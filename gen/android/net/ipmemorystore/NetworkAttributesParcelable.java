/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.ipmemorystore;
/**
 * An object to represent attributes of a single L2 network entry.
 * See NetworkAttributes.java for a description of each field. The types used in this class
 * are structured parcelable types instead of the richer types of the NetworkAttributes object,
 * but they have the same purpose. The NetworkAttributes.java file also contains the code
 * to convert the richer types to the parcelable types and back.
 * @hide
 */
public class NetworkAttributesParcelable implements android.os.Parcelable
{

  public byte[] assignedV4Address;

  public long assignedV4AddressExpiry;

  public java.lang.String groupHint;

  public android.net.ipmemorystore.Blob[] dnsAddresses;

  public int mtu;
  public static final android.os.Parcelable.Creator<NetworkAttributesParcelable> CREATOR = new android.os.Parcelable.Creator<NetworkAttributesParcelable>() {
    @Override
    public NetworkAttributesParcelable createFromParcel(android.os.Parcel _aidl_source) {
      NetworkAttributesParcelable _aidl_out = new NetworkAttributesParcelable();
      _aidl_out.readFromParcel(_aidl_source);
      return _aidl_out;
    }
    @Override
    public NetworkAttributesParcelable[] newArray(int _aidl_size) {
      return new NetworkAttributesParcelable[_aidl_size];
    }
  };
  @Override public final void writeToParcel(android.os.Parcel _aidl_parcel, int _aidl_flag)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.writeInt(0);
    _aidl_parcel.writeByteArray(assignedV4Address);
    _aidl_parcel.writeLong(assignedV4AddressExpiry);
    _aidl_parcel.writeString(groupHint);
    _aidl_parcel.writeTypedArray(dnsAddresses, 0);
    _aidl_parcel.writeInt(mtu);
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
      assignedV4Address = _aidl_parcel.createByteArray();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      assignedV4AddressExpiry = _aidl_parcel.readLong();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      groupHint = _aidl_parcel.readString();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      dnsAddresses = _aidl_parcel.createTypedArray(android.net.ipmemorystore.Blob.CREATOR);
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      mtu = _aidl_parcel.readInt();
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
