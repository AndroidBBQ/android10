/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/TextOutput.h>

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

using namespace android;

void writeString16(Parcel& parcel, const char* string)
{
    if (string != nullptr)
    {
        parcel.writeString16(String16(string));
    }
    else
    {
        parcel.writeInt32(-1);
    }
}

// get the name of the generic interface we hold a reference to
static String16 get_interface_name(sp<IBinder> service)
{
    if (service != nullptr) {
        Parcel data, reply;
        status_t err = service->transact(IBinder::INTERFACE_TRANSACTION, data, &reply);
        if (err == NO_ERROR) {
            return reply.readString16();
        }
    }
    return String16();
}

static String8 good_old_string(const String16& src)
{
    String8 name8;
    char ch8[2];
    ch8[1] = 0;
    for (unsigned j = 0; j < src.size(); j++) {
        char16_t ch = src[j];
        if (ch < 128) ch8[0] = (char)ch;
        name8.append(ch8);
    }
    return name8;
}

int main(int argc, char* const argv[])
{
    bool wantsUsage = false;
    int result = 0;
    
    while (1) {
        int ic = getopt(argc, argv, "h?");
        if (ic < 0)
            break;

        switch (ic) {
        case 'h':
        case '?':
            wantsUsage = true;
            break;
        default:
            aerr << "service: Unknown option -" << ic << endl;
            wantsUsage = true;
            result = 10;
            break;
        }
    }
#ifdef VENDORSERVICES
    ProcessState::initWithDriver("/dev/vndbinder");
#endif
    sp<IServiceManager> sm = defaultServiceManager();
    fflush(stdout);
    if (sm == nullptr) {
        aerr << "service: Unable to get default service manager!" << endl;
        return 20;
    }
    
    if (optind >= argc) {
        wantsUsage = true;
    } else if (!wantsUsage) {
        if (strcmp(argv[optind], "check") == 0) {
            optind++;
            if (optind < argc) {
                sp<IBinder> service = sm->checkService(String16(argv[optind]));
                aout << "Service " << argv[optind] <<
                    (service == nullptr ? ": not found" : ": found") << endl;
            } else {
                aerr << "service: No service specified for check" << endl;
                wantsUsage = true;
                result = 10;
            }
        }
        else if (strcmp(argv[optind], "list") == 0) {
            Vector<String16> services = sm->listServices();
            aout << "Found " << services.size() << " services:" << endl;
            for (unsigned i = 0; i < services.size(); i++) {
                String16 name = services[i];
                sp<IBinder> service = sm->checkService(name);
                aout << i 
                     << "\t" << good_old_string(name) 
                     << ": [" << good_old_string(get_interface_name(service)) << "]"
                     << endl;
            }
        } else if (strcmp(argv[optind], "call") == 0) {
            optind++;
            if (optind+1 < argc) {
                int serviceArg = optind;
                sp<IBinder> service = sm->checkService(String16(argv[optind++]));
                String16 ifName = get_interface_name(service);
                int32_t code = atoi(argv[optind++]);
                if (service != nullptr && ifName.size() > 0) {
                    Parcel data, reply;

                    // the interface name is first
                    data.writeInterfaceToken(ifName);

                    // then the rest of the call arguments
                    while (optind < argc) {
                        if (strcmp(argv[optind], "i32") == 0) {
                            optind++;
                            if (optind >= argc) {
                                aerr << "service: no integer supplied for 'i32'" << endl;
                                wantsUsage = true;
                                result = 10;
                                break;
                            }
                            data.writeInt32(atoi(argv[optind++]));
                        } else if (strcmp(argv[optind], "i64") == 0) {
                            optind++;
                            if (optind >= argc) {
                                aerr << "service: no integer supplied for 'i64'" << endl;
                                wantsUsage = true;
                                result = 10;
                                break;
                            }
                            data.writeInt64(atoll(argv[optind++]));
                        } else if (strcmp(argv[optind], "s16") == 0) {
                            optind++;
                            if (optind >= argc) {
                                aerr << "service: no string supplied for 's16'" << endl;
                                wantsUsage = true;
                                result = 10;
                                break;
                            }
                            data.writeString16(String16(argv[optind++]));
                        } else if (strcmp(argv[optind], "f") == 0) {
                            optind++;
                            if (optind >= argc) {
                                aerr << "service: no number supplied for 'f'" << endl;
                                wantsUsage = true;
                                result = 10;
                                break;
                            }
                            data.writeFloat(atof(argv[optind++]));
                        } else if (strcmp(argv[optind], "d") == 0) {
                            optind++;
                            if (optind >= argc) {
                                aerr << "service: no number supplied for 'd'" << endl;
                                wantsUsage = true;
                                result = 10;
                                break;
                            }
                            data.writeDouble(atof(argv[optind++]));
                        } else if (strcmp(argv[optind], "null") == 0) {
                            optind++;
                            data.writeStrongBinder(nullptr);
                        } else if (strcmp(argv[optind], "intent") == 0) {
                        	
                        	char* action = nullptr;
                        	char* dataArg = nullptr;
                        	char* type = nullptr;
                        	int launchFlags = 0;
                        	char* component = nullptr;
                        	int categoryCount = 0;
                        	char* categories[16];
                        	
                        	char* context1 = nullptr;
                        	
                            optind++;
                            
                        	while (optind < argc)
                        	{
                        		char* key = strtok_r(argv[optind], "=", &context1);
                        		char* value = strtok_r(nullptr, "=", &context1);
                                
                                // we have reached the end of the XXX=XXX args.
                                if (key == nullptr) break;
                        		
                        		if (strcmp(key, "action") == 0)
                        		{
                        			action = value;
                        		}
                        		else if (strcmp(key, "data") == 0)
                        		{
                        			dataArg = value;
                        		}
                        		else if (strcmp(key, "type") == 0)
                        		{
                        			type = value;
                        		}
                        		else if (strcmp(key, "launchFlags") == 0)
                        		{
                        			launchFlags = atoi(value);
                        		}
                        		else if (strcmp(key, "component") == 0)
                        		{
                        			component = value;
                        		}
                        		else if (strcmp(key, "categories") == 0)
                        		{
                        			char* context2 = nullptr;
                        			categories[categoryCount] = strtok_r(value, ",", &context2);
                        			
                        			while (categories[categoryCount] != nullptr)
                        			{
                        				categoryCount++;
                        				categories[categoryCount] = strtok_r(nullptr, ",", &context2);
                        			}
                        		}
                                
                                optind++;
                        	} 
                        	
                            writeString16(data, action);
                            writeString16(data, dataArg);
                            writeString16(data, type);
                       		data.writeInt32(launchFlags);
                            writeString16(data, component);
                        	
                            if (categoryCount > 0)
                            {
                                data.writeInt32(categoryCount);
                                for (int i = 0 ; i < categoryCount ; i++)
                                {
                                    writeString16(data, categories[i]);
                                }
                            }
                            else
                            {
                                data.writeInt32(0);
                            }                            
  
                            // for now just set the extra field to be null.
                       		data.writeInt32(-1);
                        } else {
                            aerr << "service: unknown option " << argv[optind] << endl;
                            wantsUsage = true;
                            result = 10;
                            break;
                        }
                    }
                    
                    service->transact(code, data, &reply);
                    aout << "Result: " << reply << endl;
                } else {
                    aerr << "service: Service " << argv[serviceArg]
                        << " does not exist" << endl;
                    result = 10;
                }
            } else {
                if (optind < argc) {
                    aerr << "service: No service specified for call" << endl;
                } else {
                    aerr << "service: No code specified for call" << endl;
                }
                wantsUsage = true;
                result = 10;
            }
        } else {
            aerr << "service: Unknown command " << argv[optind] << endl;
            wantsUsage = true;
            result = 10;
        }
    }
    
    if (wantsUsage) {
        aout << "Usage: service [-h|-?]\n"
                "       service list\n"
                "       service check SERVICE\n"
                "       service call SERVICE CODE [i32 N | i64 N | f N | d N | s16 STR ] ...\n"
                "Options:\n"
                "   i32: Write the 32-bit integer N into the send parcel.\n"
                "   i64: Write the 64-bit integer N into the send parcel.\n"
                "   f:   Write the 32-bit single-precision number N into the send parcel.\n"
                "   d:   Write the 64-bit double-precision number N into the send parcel.\n"
                "   s16: Write the UTF-16 string STR into the send parcel.\n";
//                "   intent: Write and Intent int the send parcel. ARGS can be\n"
//                "       action=STR data=STR type=STR launchFlags=INT component=STR categories=STR[,STR,...]\n";
        return result;
    }
    
    return result;
}

