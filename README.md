## 阅读源码的正确姿势

对于Android开发人员阅读Android系统源码是非常重要的事情，但很多时候我们关心的只是framework层的源码。

但是网上一搜全是教我们怎样把Android源码通过AOSP先下载到本地，然后进行编译，然后再导入到AS中进行调试的。这个过程是一个非常痛苦的过程，首先源码体积非常大，下载非常困难，占的磁盘空间也非常大。然后编译的话必须要有linux环境。即使编译好了由于源码太多，导入过程也是一个非常耗时间的过程。这个过程谁经历过谁知道。

为了解决上面的问题，让我们能够丝滑的在AS中阅读上framework层源码。我在将Android10源码编译完成后，将我们不关心的源码全部删除掉，**只保留了framework层的源码**，然后对其中的对大文件，无用文件进行移除。这样就将整个包体积降到了1G一下，文件数量也减少了到了3万多个。

我们开发者只需要将我项目中的源码下载下来，然后再经过简单的配置，就可以在Androd Studio 上非常丝滑的阅读FrameWork源码了，跳转，搜索、写注释什么的都是没任何问题的。😄😄

### 使用

1. 将项目中的代码下载下来
2. 打开AS，通过 Open an Existing Project 打开 android10 下面的 android.ipr文件

3. 将源码只关联本地，将dependecies下面的只留下下面两个。

   ![20210606132744.jpg](https://i.loli.net/2021/06/06/Z8OjJ7zkERHc59G.jpg)

成功导入之后，就可以愉快的看源码了，速度还是挺快的，如果感觉还是有点卡顿的话，可以将AS安装目录下的的studio.vmoptions调大一些。

```
-Xms2048m
-Xmx4049m
-XX:ReservedCodeCacheSize=500m
-XX:+UseG1GC
-XX:SoftRefLRUPolicyMSPerMB=50
-XX:CICompilerCount=3
```

## 更多

如果需要阅读其他模块的源码的话，可以将整个Android10的源码下载下来，然后将对应的模块拷贝到项目android10目录下，并在android.iml中对应的模块移除配置删掉。比如加入了packages模块，然后将下面这行删除。

```
<excludeFolder url="file://$MODULE_DIR$/packages" />
```

Android10整个源码

链接:https://pan.baidu.com/s/1V-stMpLw3xs9k2Zx7hvv8Q  密码:ifq4

如果有喜欢折腾的，需要编译整个系统源码，并调试运行整个系统的。可以参考这个 [AndroidStudio源码开发环境搭建](http://gityuan.com/2016/08/13/android-os-env/) ,过程有些复杂。

编译源码容易遇到编译出错的问题。如果遇到下面问题的话，可以考虑如下解决

1.用于初始化环境变量

```shell
soruce build/envsetup.sh  
```
2.生成 idegen.jar 文件

这个可能会生成失败，如果生成失败的话，可以通过下面的链接将idea.jar下载到本地，然后将idea.jar拷贝到 out/host/linux-x86/framework/idegen.jar ，再执行下面的命令
链接:https://pan.baidu.com/s/17lfmVAs8HRrsk5hbyzkNjA  密码:2gbx

```shell
mmm development/tools/idegen/  
```
3. 源码根目录生成文件android.ipr(工程相关设置), android.iml(模块相关配置)

```shell
./development/tools/idegen/idegen.sh
```
4. 生成了源码后，就可以通过AS打开android.ipr了，⚠️注意导入整个android源码会非常卡顿，除非必要，否则还是建议只导入自己需要的源码部分。相关的导入配置，在 android.iml中，可以根据需要通过 `excludeFolder`  在android.iml文件中将不需要的模块移除掉。我的项目中的配置如下。

```xml
 <excludeFolder url="file://$MODULE_DIR$/.repo"/>
   <excludeFolder url="file://$MODULE_DIR$/abi"/>
   <excludeFolder url="file://$MODULE_DIR$/frameworks/base/docs"/>
   <excludeFolder url="file://$MODULE_DIR$/art"/>
   <excludeFolder url="file://$MODULE_DIR$/bionic"/>
   <excludeFolder url="file://$MODULE_DIR$/bootable"/>
   <excludeFolder url="file://$MODULE_DIR$/build"/>
   <excludeFolder url="file://$MODULE_DIR$/cts"/>
   <excludeFolder url="file://$MODULE_DIR$/dalvik"/>
   <excludeFolder url="file://$MODULE_DIR$/developers"/>
   <excludeFolder url="file://$MODULE_DIR$/development"/>
   <excludeFolder url="file://$MODULE_DIR$/device"/>
   <excludeFolder url="file://$MODULE_DIR$/docs"/>
   <excludeFolder url="file://$MODULE_DIR$/external"/>
   <excludeFolder url="file://$MODULE_DIR$/hardware"/>
   <excludeFolder url="file://$MODULE_DIR$/kernel-3.18"/>
   <excludeFolder url="file://$MODULE_DIR$/libcore"/>
   <excludeFolder url="file://$MODULE_DIR$/libnativehelper"/>
   <excludeFolder url="file://$MODULE_DIR$/ndk"/>
   <excludeFolder url="file://$MODULE_DIR$/out"/>
   <excludeFolder url="file://$MODULE_DIR$/pdk"/>
   <excludeFolder url="file://$MODULE_DIR$/platform_testing"/>
   <excludeFolder url="file://$MODULE_DIR$/prebuilts"/>
   <excludeFolder url="file://$MODULE_DIR$/rc_projects"/>
   <excludeFolder url="file://$MODULE_DIR$/sdk"/>
   <excludeFolder url="file://$MODULE_DIR$/system"/>
   <excludeFolder url="file://$MODULE_DIR$/tools"/>
   <excludeFolder url="file://$MODULE_DIR$/trusty"/>
   <excludeFolder url="file://$MODULE_DIR$/vendor"/>
   //注意上面的这些移除文件的配置放到 </content> 标志的上面
   </content>
```



### 如果本文可以帮助到您，麻烦点个star吧！🙏🙏🙏

