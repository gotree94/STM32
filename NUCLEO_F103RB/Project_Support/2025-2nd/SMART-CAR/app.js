const express  = require('express');

const app      = express();

var http     = require('http').Server(app);
var io       = require('socket.io')(http);

const path = require('path');


const SerialPort = require('serialport').SerialPort;

var ReadlineParser = require('@serialport/parser-readline').ReadlineParser;

var parsers    = SerialPort.parsers;

const sp = new SerialPort( {

  path:'COM10',

  baudRate: 115200
});

 

const port = 3000;

const parser = sp.pipe(new ReadlineParser({ delimiter: '\r\n' }));

sp.on('open', () => console.log('Port open'));

parser.on('data', function (data) {
  var rcv = data.toString();

  if (rcv.substring(0, 2) == 'RD'){
	  var dist = parseInt(rcv.substring(2));
	  console.log('RDistance:',dist);
	  io.emit('RDistance', dist);
  }
  
  if (rcv.substring(0, 2) == 'LD'){
	  var dist1 = parseInt(rcv.substring(2));
	  console.log('LDistance:',dist1);
	  io.emit('LDistance', dist1);
  }
  
  //if (rcv.substring(0, 5) == 'count'){
	  //var ct = parseInt(rcv.substring(5));
	  //console.log('counting:',ct);
	  
  //}
  
});
 

app.get('/up',function(req,res)

{

	sp.write('w\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: front');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	});

});

 

app.get('/down',function(req,res)

{

	sp.write('s\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: back');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

app.get('/left',function(req,res)

{

	sp.write('a\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: left');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});


app.get('/right',function(req,res)

{

	sp.write('d\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: right');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

app.get('/clear',function(req,res)

{

	sp.write(' \n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: clear');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

app.get('/1',function(req,res)

{

	sp.write('1\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: Level 1');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

app.get('/2',function(req,res)

{

	sp.write('2\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: Level 2');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

app.get('/3',function(req,res)

{

	sp.write('3\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: Level 3');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

app.get('/4',function(req,res)

{

	sp.write('4\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: Level 4');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

app.get('/0',function(req,res)

{

	sp.write('0\n\r', function(err){

		if (err) {

            return console.log('Error on write: ', err.message);

        }

        console.log('send: Level 0');

		res.writeHead(200, {'Content-Type': 'text/plain'});

		res.end('\r');

	}); 

});

 

app.use(express.static(__dirname + '/public'));

 

http.listen(port, function () {
  console.log('Server listening on http://localhost:' + port);
});
