var express = require('express');
var app = express();

var phpExpress = require('php-express')({
  binPath: 'php'
});

// set view engine to php-express
app.set('views', './public');
app.engine('php', phpExpress.engine);
app.set('view engine', 'php');

// routing all .php file to php-express
app.all(/.+\.php$/, phpExpress.router);

var server = app.listen(3000, function () {
  console.log('listening on *:3000');
});