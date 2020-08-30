#!/bin/bash
#Run MongoDB

mdir -p ${HOME}/mongodb/data
mongod --fork --dbpath ${HOME}/mongodb/data --logpath ${HOME}/mongodb/mongodb/mongod.log --port 12345