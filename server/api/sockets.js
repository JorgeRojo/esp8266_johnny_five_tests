const io = require('socket.io')();



io.on('connection', client => {
  console.log("socket.io connection");
 
  client.on('face', data => {
    console.warn('>>>--------> face ', data);
    client.emit('face', data);
  });
 

});

module.exports = io;