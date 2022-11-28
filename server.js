const express = require("express");
const moment = require('moment');
const app = express();
const PORT =  4200;
const bodyParser = require('body-parser')
let contCajas = 1;
app.use(express.json());
app.use(bodyParser.json())
app.use(express.urlencoded({extended:true}));

let arrValues = [];
app.get("/", async (req, res) => {
    console.log(arrValues)
    res.status(200).json(arrValues)
});
app.post("/post", (req, res) => {
    let {message,time} = req.body; 
    if(message === "start"){
      let obj = {
        caja: contCajas
      };
      obj["start"] = time;
      arrValues.push(obj);     
    }
    else if (message="finish"){
      arrValues.forEach(el => {
          if(el.caja === contCajas){
            el["finish"] = time;
            el["duracion"] = calcTiempo(el.start,el.finish);
          } 
      });
      contCajas++;
    }   
    console.log(arrValues)
    res.status(201).send({message:"Data Enviada"});
})
app.listen(PORT, () => console.log("Listening on port", PORT));

function calcTiempo(start,finish) {
  let inicio = start.split(':').map(el => parseInt(el));
  let fin = finish.split(':').map(el => parseInt(el));
  inicio = moment({h:inicio[0],m:inicio[1],s:inicio[2]});
  fin = moment({h:fin[0],m:fin[1],s:fin[2]});
  let seconds = fin.diff(inicio,'seconds')
  let hour = Math.floor(seconds / 3600);
  hour = (hour < 10)? '0' + hour : hour;
  let minute = Math.floor((seconds / 60) % 60);
  minute = (minute < 10)? '0' + minute : minute;
  let second = seconds % 60;
  second = (second < 10)? '0' + second : second;
  let str = hour + ':' + minute + ':' + second;
  return str
}