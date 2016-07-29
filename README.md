# What is anyserver?
anyserver is a component library to support intercommunication between local processes and also remote. <br>
Like websocket, http, inet domain for external communication and also unix domain server for IPC.

# Architecture

# How to configure

# How to build
1. Create build directory
<pre>
#mkdir build
#cd build
</pre>
2. Build as debug
<pre><code>
#cmake .. -Dbuild=debug 
</code></pre>
3. Build as release
<pre><code>
#cmake .. -Dbuild=release
</code></pre>
4. Example executable and anyserver library deploy in build/out
<pre>
#cd build/out
#./example anyserver.json
</pre>

# How to port this library

# External Dependencies
jsoncpp
libwebsockets

# Example



