(function(t){function e(e){for(var a,l,r=e[0],s=e[1],c=e[2],d=0,f=[];d<r.length;d++)l=r[d],Object.prototype.hasOwnProperty.call(i,l)&&i[l]&&f.push(i[l][0]),i[l]=0;for(a in s)Object.prototype.hasOwnProperty.call(s,a)&&(t[a]=s[a]);u&&u(e);while(f.length)f.shift()();return o.push.apply(o,c||[]),n()}function n(){for(var t,e=0;e<o.length;e++){for(var n=o[e],a=!0,r=1;r<n.length;r++){var s=n[r];0!==i[s]&&(a=!1)}a&&(o.splice(e--,1),t=l(l.s=n[0]))}return t}var a={},i={app:0},o=[];function l(e){if(a[e])return a[e].exports;var n=a[e]={i:e,l:!1,exports:{}};return t[e].call(n.exports,n,n.exports,l),n.l=!0,n.exports}l.m=t,l.c=a,l.d=function(t,e,n){l.o(t,e)||Object.defineProperty(t,e,{enumerable:!0,get:n})},l.r=function(t){"undefined"!==typeof Symbol&&Symbol.toStringTag&&Object.defineProperty(t,Symbol.toStringTag,{value:"Module"}),Object.defineProperty(t,"__esModule",{value:!0})},l.t=function(t,e){if(1&e&&(t=l(t)),8&e)return t;if(4&e&&"object"===typeof t&&t&&t.__esModule)return t;var n=Object.create(null);if(l.r(n),Object.defineProperty(n,"default",{enumerable:!0,value:t}),2&e&&"string"!=typeof t)for(var a in t)l.d(n,a,function(e){return t[e]}.bind(null,a));return n},l.n=function(t){var e=t&&t.__esModule?function(){return t["default"]}:function(){return t};return l.d(e,"a",e),e},l.o=function(t,e){return Object.prototype.hasOwnProperty.call(t,e)},l.p="/webusb/";var r=window["webpackJsonp"]=window["webpackJsonp"]||[],s=r.push.bind(r);r.push=e,r=r.slice();for(var c=0;c<r.length;c++)e(r[c]);var u=s;o.push([0,"chunk-vendors"]),n()})({0:function(t,e,n){t.exports=n("56d7")},"56d7":function(t,e,n){"use strict";n.r(e);n("e260"),n("e6cf"),n("cca6"),n("a79d");var a=n("2b0e"),i=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("v-app",[a("v-app-bar",{staticClass:"cronologic-app-bar",attrs:{app:""}},[a("div",{staticClass:"d-flex align-center"},[a("v-img",{staticClass:"shrink mt-1",attrs:{alt:"cronologic Logo",contain:"",src:n("5953"),width:"100"}})],1),a("div",{staticClass:"d-flex align-center ml-8"},[a("v-toolbar-title",[t._v("WebUSB")])],1),a("v-spacer"),a("div",{staticClass:"d-flex align-center"},[a("v-switch",{attrs:{"append-icon":"mdi-usb",color:t.deviceState===t.DEVICE_CONNECTING?"warning":"success","hide-details":"",loading:t.deviceState===t.DEVICE_CONNECTING},model:{value:t.deviceToggle,callback:function(e){t.deviceToggle=e},expression:"deviceToggle"}})],1)],1),a("v-main",{staticClass:"mt-4"},[a("v-container",[a("v-row",[a("v-col",{attrs:{cols:"12",sm:"6","offset-sm":"3",md:"4","offset-md":"4"}},[a("ClockConfig",{attrs:{disabled:!t.connected}})],1)],1),a("v-row",t._l(t.channelConfig,(function(e){return a("v-col",{key:e.id,attrs:{cols:"12",sm:"6",md:"4",lg:"3"}},[a("ChannelConfig",{attrs:{id:e.id,color:e.color,disabled:!t.connected}})],1)})),1)],1),a("v-snackbar",{attrs:{timeout:5e3},model:{value:t.connectionFakeNotification,callback:function(e){t.connectionFakeNotification=e},expression:"connectionFakeNotification"}},[a("v-icon",{attrs:{color:"light-blue"}},[t._v("mdi-information")]),t._v(" The “connect” toggle is currently just a mockup. No USB connection is made. ")],1)],1)],1)},o=[],l=function(){var t=this,e=t.$createElement,n=t._self._c||e;return n("v-card",{attrs:{outlined:"",disabled:t.disabled}},[n("v-card-title",{class:t.color+" lighten-4"},[t._v(" Channel "+t._s(t.id)+" ")]),n("v-list",[n("v-list-item",[n("v-list-item-content",[n("v-list-item-title",[t._v("Multiplier")]),n("p",{staticClass:"text-caption text--secondary"},[t._v(" Set the number of pulses per clock cycle as a fraction, i.e. "),n("sup",[t._v("numerator")]),t._v("/"),n("sub",[t._v("denominator")])]),n("v-row",[n("v-col",[n("v-text-field",{staticClass:"no-spinbox",attrs:{label:"Numerator",type:"number",rules:t.num_rules},model:{value:t.num,callback:function(e){t.num=e},expression:"num"}})],1),n("v-col",[n("v-text-field",{staticClass:"no-spinbox",attrs:{label:"Denominator",type:"number",rules:t.denom_rules},model:{value:t.denom,callback:function(e){t.denom=e},expression:"denom"}})],1)],1)],1)],1),n("v-divider"),n("v-list-item",[n("v-list-item-content",[n("v-row",[n("v-col",[n("v-text-field",{staticClass:"no-spinbox",attrs:{label:"Phase",type:"number",suffix:"ns",rules:t.phase_rules},model:{value:t.phase_ns,callback:function(e){t.phase_ns=e},expression:"phase_ns"}})],1),n("v-col",[n("v-text-field",{staticClass:"no-spinbox",attrs:{label:"Pulse width",type:"number",suffix:"ns",rules:t.width_rules},model:{value:t.width_ns,callback:function(e){t.width_ns=e},expression:"width_ns"}})],1)],1)],1)],1)],1)],1)},r=[];n("99af"),n("ac1f"),n("1276"),n("466d"),n("a9e3");function s(t){return function(e){return String(e).length<=t||"".concat(t," digit").concat(1===t?"":"s"," max")}}function c(t){return function(e){return e=String(e).split(".",2),!(2===e.length&&e[1].length>t)||"".concat(t," fractional digits max")}}function u(t){return String(t).match(/^[0-9]+$/)?"0"!==String(t)||"cannot be zero":"only positive integers"}function d(t){return!!String(t).match(/^[0-9.]+$/)||"only positive numbers"}var f={name:"ChannelConfig",data:function(){return{num:this.numerator,denom:this.denominator,phase_ns:this.phase,width_ns:this.width,num_rules:[u,s(2)],denom_rules:[u,s(1)],phase_rules:[d,c(5),function(t){return t=parseFloat(t),1e6*t%250!==0?"only multiples of 250 fs":!(t>1e6)||"max 1 ms"}],width_rules:[d,c(3)]}},props:{color:{type:String,default:"grey"},disabled:{type:Boolean,default:!1},id:{type:Number,required:!0},numerator:{type:Number,default:1},denominator:{type:Number,default:1},phase:{type:Number,default:123.45625},width:{type:Number,default:123.456}}},p=f,v=n("2877"),m=n("6544"),b=n.n(m),h=n("b0af"),g=n("99d9"),C=n("62ad"),_=n("ce7e"),V=n("8860"),y=n("da13"),E=n("5d23"),x=n("0fd9"),w=n("8654"),S=Object(v["a"])(p,l,r,!1,null,null,null),N=S.exports;b()(S,{VCard:h["a"],VCardTitle:g["a"],VCol:C["a"],VDivider:_["a"],VList:V["a"],VListItem:y["a"],VListItemContent:E["a"],VListItemTitle:E["b"],VRow:x["a"],VTextField:w["a"]});var k=function(){var t=this,e=t.$createElement,n=t._self._c||e;return n("v-card",{attrs:{outlined:"",disabled:t.disabled}},[n("v-card-title",{staticClass:"grey lighten-3"},[t._v(" Base clock ")]),n("v-list",[n("v-list-item",[n("v-list-item-content",[n("v-row",[n("v-col",{attrs:{cols:"8"}},[n("v-text-field",{staticClass:"no-spinbox",attrs:{label:"Value",type:"number",rules:t.nanoRules},model:{value:t.nanoVal,callback:function(e){t.nanoVal=e},expression:"nanoVal"}})],1),n("v-col",{attrs:{cols:"4"}},[n("v-select",{attrs:{items:t.clockUnits,label:"Unit"},model:{value:t.unitVal,callback:function(e){t.unitVal=e},expression:"unitVal"}})],1)],1)],1)],1)],1)],1)},I=[],D=["kHz","MHz","ns","µs"],T={name:"ClockConfig",data:function(){return{clockUnits:D,nanoVal:this.nanos,unitVal:this.unit,nanoRules:[function(t){return!String(t).match(/-/)||"no negative numbers"},function(t){return!!String(t).match(/^[0-9.]+$/)||"only digits and decimal point allowed"}]}},props:{disabled:{type:Boolean,default:!1},nanos:{type:Number,default:123456789},unit:{type:String,default:"ns",validator:function(t){return-1!==D.indexOf(t)}}}},O=T,j=n("b974"),L=Object(v["a"])(O,k,I,!1,null,null,null),B=L.exports;b()(L,{VCard:h["a"],VCardTitle:g["a"],VCol:C["a"],VList:V["a"],VListItem:y["a"],VListItemContent:E["a"],VRow:x["a"],VSelect:j["a"],VTextField:w["a"]});var P=0,F=1,A=2,M={name:"App",components:{ChannelConfig:N,ClockConfig:B},data:function(){return{channelConfig:[{id:1,color:"red"},{id:2,color:"orange"},{id:3,color:"yellow"},{id:4,color:"lime"},{id:5,color:"green"},{id:6,color:"teal"},{id:7,color:"blue"},{id:8,color:"purple"}],connectionFakeNotification:!1,deviceState:P}},computed:{connected:function(){return this.deviceState===this.DEVICE_CONNECTED},deviceToggle:{get:function(){return this.deviceState!==this.DEVICE_DISABLED},set:function(t){var e=this;t?this.deviceState==this.DEVICE_DISABLED&&(this.deviceState=this.DEVICE_CONNECTING,this.connectionFakeNotification=!0,setTimeout((function(){e.deviceState===e.DEVICE_CONNECTING&&(e.deviceState=e.DEVICE_CONNECTED)}),1e3)):this.deviceState=this.DEVICE_DISABLED}}},created:function(){this.DEVICE_DISABLED=P,this.DEVICE_CONNECTING=F,this.DEVICE_CONNECTED=A}},$=M,G=(n("5c0b"),n("7496")),R=n("40dc"),U=n("a523"),z=n("132d"),H=n("adda"),J=n("f6c4"),q=n("2db4"),W=n("2fa4"),K=n("b73d"),Q=n("2a7f"),X=Object(v["a"])($,i,o,!1,null,null,null),Y=X.exports;b()(X,{VApp:G["a"],VAppBar:R["a"],VCol:C["a"],VContainer:U["a"],VIcon:z["a"],VImg:H["a"],VMain:J["a"],VRow:x["a"],VSnackbar:q["a"],VSpacer:W["a"],VSwitch:K["a"],VToolbarTitle:Q["a"]});var Z=n("f309");a["a"].use(Z["a"]);var tt=new Z["a"]({});a["a"].config.productionTip=!1,new a["a"]({vuetify:tt,render:function(t){return t(Y)}}).$mount("#app")},5953:function(t,e,n){t.exports=n.p+"img/cronologic_logo_2020.e42c9234.png"},"5c0b":function(t,e,n){"use strict";n("9c0c")},"9c0c":function(t,e,n){}});
//# sourceMappingURL=app.e005e9c0.js.map