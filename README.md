# symbios

# Dependencies

## Redis

### Hireds

A C library for interacting with Redis. Redis-plus-plus depends on it.

> wget https://github.com/redis/hiredis/archive/v1.0.0.tar.gz  
> tar -xvzf v1.0.0.tar.gz  
> cd hiredis*  
> make  PREFIX=/path/to/hiredis/install  
> sudo make PREFIX=/path/to/hiredis/install install   

### Redis-Plus-Plus

A library for interacting with a Redis database in C++, located [here](https://github.com/sewenew/redis-plus-plus)

> wget https://github.com/sewenew/redis-plus-plus/archive/1.1.2.tar.gz      
> tar -xvzf 1.1.2.tar.gz      
> cd redis-plus-plus*    
> mkdir build  
> cd build  
> cmake -DCMAKE_PREFIX_PATH=/path/to/hiredis -DCMAKE_INSTALL_PREFIX=/path/to/install/redis-plus-plus -DCMAKE_BUILD_TYPE=Release ../  
> make  
> sudo make install  

## MongoDB

### Mongodb C Driver

> wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.0/mongo-c-driver-1.17.0.tar.gz  
> tar -xvzf mongo-c-driver-1.17.0.tar.gz    
> cd mongo-c-driver-1.17.0  
> mkdir build  
> cd build  
> cmake -DCMAKE_INSTALL_PREFIX="/path/to/install/mongo-C-driver" -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ../  
> make -j8  
> sudo cmake --build . --target install  

### Mongodb C++ Driver

> curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.6.0/mongo-cxx-driver-r3.6.0.tar.gz  
> tar -xzf mongo-cxx-driver-r3.6.0.tar.gz    
> cd mongo-cxx-driver-r3.6.0/build  
> cmake .. -DCMAKE_PREFIX_PATH="/path/to/mongo-C-driver" -DCMAKE_INSTALL_PREFIX="/path/to/install/mongo-C++-driver" -DCMAKE_CXX_STANDARD=17 -DBSONCXX_POLY_USE_BOOST=1 -DCMAKE_BUILD_TYPE=Release     
> make -j8  
> sudo cmake --build . --target install  
