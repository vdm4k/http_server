# HTTP server

Http server is a labrary that can be used to write your own http serverl
It's easy to start and easy to write tests ( thanks to network library )


## ***Build***

You need cmake

1. mkdir build
2. cd build
3. cmake ../
   1. cmake build options 
      1. build examples *-DWITH_EXAMPLES=ON* 
      2. with sanitazer support *-DWITH_SANITIZER=ON* 
4. make 

## Design

Http library is asynchronous multi-threaded library. It use one listen thread who's waiting for incomming connections and 
pull of handler threads who's parse/handle incomming requests.

## Examples

### minimal application


```cpp
   // create pointer on specific configuration listen  
   std::unique_ptr<bro::net::listen::settings> connection_settings(new tcp::listen::settings());
   connection_settings->_listen_address = {proto::ip::v4::address("127.0.0.1"), 23456};;

   // create server
   http::server::http_server server(std::move(connection_settings));

   // add specific handle
   server.add_handler(http::client::request::type::e_GET, "/", {._cb = [](http::server::request &&req, http::server::response &resp, std::any user_data) {
      resp.set_status_code(http::status::code::e_OK);
      resp.add_body(std::string("Hi All"), "txt");
   }});

   // start server
   if(!server.start()) {
      std::cerr << "couldn't start http server" << std::endl;
   } else {
      std::this_thread::sleep_for(std::chrono::seconds(50));
      server.stop();    
   }
```

### server with config

More complex example can be found in examples/simple_server folder.
Can be build with *-DWITH_EXAMPLES=ON* 