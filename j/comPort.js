// definitions of comPort stat objects, v 1.0.0.1

var dic_comState = [ // —ловарь перекодировани€ состо€ний ком-порта
  {n:0,c:"none",h:"com port not open"},
  {n:1,c:"initing",h:"com port initing"},
  {n:2,c:"inited",h:"com port ready to sim"},
  {n:3,c:"registred",h:"sim registred in a network and ready to use"},
  ];

var Monitor = [ // registration data for a forms
  {n:"comState",d:dic_comState,h:"comPort status"}, // dic field - needs to decode
  {n:"network",h:"network where sim registred"},
  {n:"imsi",h:"imsi of attached sim"},
  {n:"icc_id"},
  ];


var data = { // sample test data
   comState:1,
   netork:"MTS-RUS",
   imsi:"2500100002304",
   icc_id:"909239848023902",
   };