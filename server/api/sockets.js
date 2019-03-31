const io = require('socket.io')(); 

io.on('connection', client => {
  console.log("socket.io connection");
  client.on('battery', data => {
    console.warn('>>>>------------> server ', data); 
  });
  client.on('face', data => {
    console.warn('>>>>------------> face ', data);   
    client.emit('face_ok', data);
  });
  client.on('disconnect', () => { 
    console.log("socket.io disconnect");
  }); 

});

module.exports = io;