// Общие javascript функции
var xHost = null; //"http://192.168.6.60:82"; // Default Host
//alert('vs');
// -------   Функции разбора строк ---------------

function id(name) { return document.getElementById(name);}


function txt2var(t) {
var t=t.split('\n'),i,n=t[0].split('\t'),v=null;
if (t.length>1) v=t[1].split('\t');
for(i=0;i<n.length;i++) {
  //alert('set:'+n[i]); //t[0][i]);
  this[n[i]]=(v && i<v.length)?c_decode(v[i]):null;
  }
return this;
}

function getRows(s) { // Извлекает строки разделенные '\n', игнорирует '\r'
var f=0,l=s.length,p=0,a=new Array();
while(p<l) {
 var ii,i;
 for(ii=0;ii+p<l && s.charAt(ii+p)!='\n';ii++);
 i=ii; while(i>0 && s.charAt(i+p-1)=='\r') i--;
 a.push(s.substr(p,i)); // Добавляем cтроку
 ii++; p+=ii; 
 }
return a;
}




function c_encode(s) {
var i,r="";
for(i=0;i<s.length;i++) switch(s.charAt(i)) {
 case '\r': r+='\\r'; break;
 case '\n': r+='\\n'; break;
 case '\t': r+='\\t'; break;
 case '\\': r+='\\\\'; break;
 default : r+=s.charAt(i);
 }
return r;
}


function c_decode(s) {
var i,r="";
//alert('DEC:'+s);
for(i=0;i<s.length;i++) if (s.charAt(i)=='\\' && i+1<s.length) {
 i++;
 switch(s.charAt(i)) {
 case '\\': r+='\\'; break;
 case 'r': r+='\r'; break;
 case 'n': r+='\n'; break;
 case 't': r+='\t'; break;
 default : r+=s.charAt(i);
 }
 } else r+=s.charAt(i);
//alert(s);
return r;
}

/*

function c_encode(s) {
return s.replace(/\n/g,'\\n').replace(/\r/g,'\\r').replace(/\t/g,'\\t'); //.replace(/\\/g,'\\\\');
}


function c_decode(s) {
return s.replace(/\\n/g,'\n').replace(/\\r/g,'\r').replace(/\\t/g,'\t'); //.replace(/\\\\/g,'\\');
}
*/


function mkNV(a) {
var i,s="";
for(i=0;i+1<a.length;i+=2) s+=a[i]+'\t'+c_encode(a[i+1].toString())+'\n';
//alert('mkNV:'+s);
return s;
}



function chrIn(s,l) { // Есть ли символ в списке ???
for(i=0;i<l.length;i++) if (s==l.charAt(i)) return true;
return false;
}

function getWords(s,del) { // Разделители ' \t\r\n', учитываются ""''
var f=0,l=s.length,p=0,a=new Array();
if (!del) del=" \t\r\n"; // Дефолтовые делимитеры ...
while(p<l) {
 var i,ch;
 for(i=0;i+p<l && chrIn(s.charAt(i+p),del);i++);
 p+=i; i=0; if (p>=l) break; // Значит больше нету ...
 ch = s.charAt(i+p);
 if (ch =='"' || ch=="'") { // Экранированная строка ...
   //ch=s.charAt(i+p); alert('String='+ch);
   for(i=1;i+p<l;i++) if (s.charAt(i+p)==ch) { i++; break;} 
   a.push(s.substr(p+1,i-2)); // Добавляем cтроку
   }
 else {  // Обычное слово ...
  for(i=0;i+p<l && !chrIn(s.charAt(i+p),del);i++);
   a.push(s.substr(p,i)); // Добавляем cтроку
  }
 p+=i; // Сдвигаемся  
 }
return a;
}

// -- Utilities

function getParentByTag(e,name) {
while(1) { e = e.parentNode; if (!e) return null ; if (e.tagName==name) return e;}
return null;
}


// --- Асинхронный реквестер 

Function.prototype.bind = function(object) {
    var method = this
    return function() {
        return method.apply(object, arguments) 
    }
}

var Req = new Array(); // Current Requests ...
var reqNum = 1;
var iPath = ""; // Default Image Path


function aRemove(a,e) { // Remove element from array
for(i=0;i<a.length;i++) if (a[i]==e) {
 //alert('Delete!');
 a.splice(i,1);
 return;
 }
}

var oneX = null;

function vRequester(e,cb) {
//if ( !oneX ) oneX = new ActiveXObject('Msxml2.XMLHTTP')
this.x = ajax(); //new ActiveXObject('Msxml2.XMLHTTP');
         //new ActiveXObject("Microsoft.XMLHTTP");
this.e = e;
this.cb = cb;
return this;
}

function vReqX(url,data,e,cb,r) {
//alert(1);
var x, ee = new vRequester(e,cb),u,err; //{x:x,e:e,cb:cb};
x = ee.x;  Req.push(ee);
x.onreadystatechange = function() {
 var x=this.x,e=this.e;
 if (x.readyState == 4) {
   var t=x.responseText;
   t=x.responseText; 
   //x.onreadystatechange=null;
   aRemove(Req,this); t.x=0; // Remove from a list
   if (this.cb) { // Have a callback !!!
     this.cb.call(e,e,t);
     return;
     }
   alert(t);
   }
 }.bind(ee);
//alert(1);
u=xHost+url; ///db?r='+reqNum+'&sql='+url;
//alert(u);
try{
x.open("POST",u,false);
x.send(data); 
} catch(err) {  
  alert(err.description);
  aRemove(Req,ee);  ee.x=0;
  if (!r || r<=0) { alert('SendEx:'+err.description);  return null;} 
  alert('IE - глючит - Begin retry URL='+url+'DATA=\n'+data);
  return vReqX(url,data,e,cb,r-1);
  // else - try again ...
  }

return ee;
}

function vReq(url,data,e,cb) {
return vReqX(url,data,e,cb,3);
}

// --  Функции инициализации приложения -----


var it= { v:1,
  img: '/i/', // Default path to images ...
  tmpID: 1,  // TempId for elements ...
  pname: new Array(), // Parameters names
  pval:  new Array(),
  par : function (x) { var i; 
        if (!this.params) getParams(document.location.href); // Если еще не грузились -)
        for(i=0;i<this.pname.length;i++) 
        if (this.pname[i]==x) return this.pval[i]; return null; }
  }; // Версия, proto,srv,doc  p = параметры !!!

var vs = it;

function getParams(h) {
var u=h,i,p="",proto='file://',srv='';
//alert(h);
for(i=0;i<h.length;i++) if (h.charAt(i)=='?') { // Parameters block !!!
    u=h.substr(0,i-1); p=h.substr(i+1); break; }
// Отделяем протокол ...
if (u.substr(0,7)=='http://') { proto='http://'; u=u.substr(7); }
 else if (u.substr(0,7)=='file://') { proto='file://'; u=u.substr(7); }
   else { proto='file://'; }
// Теперь отделяем сервер-пас ( до /)
for(i=0;i<u.length;i++) if (u.charAt(i)=='/') {
 srv=u.substr(0,i); u = u.substr(i); break; // Url -начинается с '/'!!!
 }
it.proto=proto; it.srv=srv; it.doc=u; it.params = p.split('#')[0].split('&'); // Params as list
it.p={};  //  it.pname = new Array(); it.pval=new Array(); // pnames & pvalues
for(i=0;i<it.params.length;i++) {
 var p=it.params[i],j,n,v;
 for(j=0;j<p.length;j++) if (p.charAt(j)=='=') { 
    var v=p.substr(j+1);
    v=v.replace(/\%20/g,' ');   // ZU - временная мера...
    it.pname.push(p.substr(0,j)); it.pval.push(v); break; // URL encode ZU!
    }
 }
//alert(it.par('f'));
//if (document.location.href.split('?').length>1) { // Parameters Here !!!
}

getParams(document.location.href); // Настройка параметров ...
if (!xHost) {
 //alert(it);
 if (it.proto=='http://') xHost='http://'+it.srv; // SetDefaultHost
   else xHost='http://127.0.0.1:82';
 //alert(xHost);
 }
//alert(it.srv);


function init(e) {
var pp,i,p;

pp=document.getElementsByTagName('DIV'),i,p;
for(i=0;i<pp.length;i++) {
 p = pp[i]; // or .item(i)
 if (p.className=='dtree') {
   var c=p.first;
   if (c) { c = c.split(','); //alert(p.first);
      e=nodeCreate(p,p,c[0],c[1],c[2]); //'class.gif','First');
      if (p.open) nodeClicked(e.childNodes[0]);
      }
   }
 }
pp=document.getElementsByTagName('pre');
for(i=0;i<pp.length;i++) {
 p = pp[i]; // or .item(i)
 //if (p.className=='vForm') p2form(p);  // Convert to forms !!!
 if (0);
 else if (p.className=='vTab') p2tab(p); // Convert to tabs !!!
 else if (p.className=='vAct') p2act(p); // Convert to actions !!!
 else if (p.className.substr(0,4)=='note') elem2note(p);
 }
if (window.OnPostLoad) setTimeout("OnPostLoad()",100);
}

var root = window.addEventListener || window.attachEvent ? window : document.addEventListener ? document : null;
if (root){
    if (root.addEventListener) root.addEventListener("load", init, false);
    else if (root.attachEvent) root.attachEvent("onload", init);
    }


// Ajax Utilities 

function ajax() {
var r=null,e=null;
if (document.all) {
try {  r = new ActiveXObject("Msxml2.XMLHTTP"); } 
catch (e) { 
   try { r  = new ActiveXObject("Microsoft.XMLHTTP"); }
   catch(e) { 
      try { r = new  XMLHttpRequest(); }
      catch(e) {}
      }
   }
} else r = new XMLHttpRequest();
if (!r) alert('noAjax:'+e);
return r;
}


function vReqText(sql,url) {
var r = null, txt="";
if (!url) url='/db/s';
r =  ajax(); // Create ajax engine
if (!r) return null;
try{
r.open("POST",url,false); //false);
r.send(sql);
} catch(e) { return "-"+e.description; }
//alert('Ajax:'+r.responseText); //txt+' on '+sql);  
return r.responseText;
}


// -- decoders for Services stub

function txt2struct(txt) {
var r = getRows(txt), c;
for(i=0;i<r.length;i++) {
   c = r[i].split('\t');
   if (c.length<2) continue;
   this[c[0]]=c_decode(c[1]);
  }
return this;                
}

function txt2array(txt) {
var r = getRows(txt),a; var i,j;
c = r[0].split('\t'); // Column Name
a  = [];
for(i=1;i<r.length;i++) { // RowByRow
 var v = r[i].split('\t'),vv={}; // Values
 for(j=0;j<v.length && j<c.length;j++)
    vv[c[j]]=c_decode(v[j]);
 a.push(vv); 
 }
return a;
}


function srvCall(fun,typ, par) {
var i;
fun='/srv?FUN='+fun;
for(i=0;i<par.length-1;i+=2) fun+='&'+par[i]+'='+ escape(par[i+1]);  // UrlEncode?
//alert('BeginCall:'+fun);
fun = vReqText(null,fun); // Exchange data
if (fun.charAt(0)=='-') {
  throw fun; // Exception ...
  return;
  }
//alert('ok?');
switch(typ) {
case 0: return fun.charAt(0)=='+'; // Status OK?
case 1: return fun; // SingleValue
case 2: return new txt2struct(fun); // PageBook
case 3:  {  return txt2array(fun);}
}
alert('RetType:'+typ+' unknown!');
return 0;
}





/// -- notes reFormat

function txtNote(s,caption,text) {
if (!s) s = ["#edc229","#ffffcc"];
if (!caption) caption='';
if (!text) text='';
var str="<table class=note cellpadding=0 cellspacing=0 width=100%>";
str+="<tr><th align=left style='background-color:"+s[0]+
 ";background-image:url(/i/noteRight.gif);background-position:top right;background-repeat: no-repeat;"+
 "'>&nbsp "+caption+"<td style='background-color:white;background-image:none' width=100>";
str+="<tr><td colspan=2 style='background-color:"+s[1]+";border: 1px ridge rgb(51, 153, 102);'>"+
     text+"</table>";
//alert(str);
//document.write(str);
return str;
}

function elem2note(e) { // Љ®­ўҐавЁа®ў вм PRE ў § ¬ҐвЄг
var div,td,s;
div = document.createElement('DIV');
s = e.className.split('.'); // ‚бҐ Єа®¬Ґ note
if (s.length==3) {  s[0]= '#'+s[1]; s[1]='#'+s[2];  } 
            else s=null;
document.body.insertBefore(div,e); //,"TEST");
div.innerHTML=txtNote(s,e.title,""); //'Here?';
td=div.getElementsByTagName('TR')[1].getElementsByTagName('TD')[0];
td.insertBefore(e,null); // AttchPre here 
//alert(td.innerHTML);

}



/// ---- Nodes

function getRows2(txt) { // ‚лв бЄЁў Ґв бва®ЄЁ - б гзҐв®¬ ®вбвгЇ . …б«Ё ­Ґв ®вбвгЇ  - бва®Є  Їа®¤®«¦ Ґвбп
var L = txt.length,p=0,res=new Array();
//alert(L);
while(p<L) {
 var l,sl;
 for(l=0;p+l<L;l++) if (txt.charAt(p+l)=='\n' && txt.charAt(p+l+1)==' ') { l++; break;} // „«Ё­г
 sl=l; if (txt.charAt(p+sl-1)=='\n') sl--; if (txt.charAt(p+sl-1)=='\r') sl--;
 res.push(txt.substr(p,sl));
 p+=l;
 }
return res;
}

function identLevel(t) { // Define number of levels
var i;
 for(i=0;i<t.length;i++) if (t.charAt(i)!=' ') break; // done level
return i;
}

function row2nodes(node, row, p, L ) { // „®Ў ў«пҐ¬ ¤ҐвҐ© ў ¬ ббЁў Ї® ЇҐаў®¬г н«Ґ¬Ґ­вг
var lev,cnt=0,last,k;
for( lev=identLevel( row[p] ); p<L; cnt+=k, p+=k)  { // Collect rows with same or more level
     var l = identLevel(row[p]);
     if (l<lev) break; // done - next level
     if (l==lev) { // this level -)
          last = { name:row[p].substr(lev) }; // ’ Є п ­®ў п бвагЄвга 
          if (!node.item) node.item = new Array();
          node.item.push( last ); // Add Empty Push Object 
          k=1;
          }
     else k=row2nodes( last , row, p, L); // child level
     }
return cnt; // ‘Є®«мЄ® г¤ «®бм § Ўа вм -)
}

function txt2nodes(txt) { // ЇҐаҐзЁб«пҐ¬ ўе®¤­®© ¬ ббЁў ў ­ Ў®а бвагЄвга
var row=getRows2(txt), L=row.length, i, node={}; 
for( p=0; p<L; p+=i) i = row2nodes(node, row, p, L ); // ‘Є®«мЄ® г¤ «®бм ®вЄгбЁвм ???
return node; 
}

function nodes2list(node,fmt) {
var txt='',it = node.item, i;
if (!it) return '';
for(i=0;i<it.length;i++) {
  txt+=fmt?fmt(it[i]):'<li>'+it[i].name; // def node formatter -)
  if (it[i].item) txt+=nodes2list(it[i],fmt);
  }
return '<UL>'+txt+'</UL>';
}
