import { useState } from “react”;

const uid = () => Math.random().toString(36).slice(2, 8);

const C = {
celeste:     “#a6d7d9”,
celesteDark: “#498f91”,
celesteLight:”#d4eef0”,
oliva:       “#d5d06a”,
olivaDark:   “#a8a618”,
durazno:     “#ecc6a3”,
duraznoDeep: “#b1854f”,
crema:       “#ede8d0”,
cremaDeep:   “#ddd8be”,
ink:         “#1d1d1b”,
inkLight:    “#4a4a47”,
inkFaint:    “#9a9a96”,
white:       “#ffffff”,
danger:      “#c0392b”,
};

const ALERGENOS_ES = {
gluten:“Gluten”, lacteos:“Lácteos”, huevos:“Huevos”,
frutos_secos:“Frutos secos”, mani:“Maní”, soja:“Soja”,
sesamo:“Sésamo”, mostaza:“Mostaza”, apio:“Apio”,
moluscos:“Moluscos”, crustaceos:“Crustáceos”, pescado:“Pescado”,
sulfitos:“Sulfitos”, altramuces:“Altramuces”,
};

const CATEGORIAS_RECETA = [“Crema”,“Fruta”,“Sorbete”,“Vegano”,“Yogur”,“Especial”];
const CAT_EMOJI = {Crema:“🍦”,Fruta:“🍓”,Sorbete:“🍧”,Vegano:“🌱”,Yogur:“🥛”,Especial:“⭐”};

const emptyIng = () => ({
id:uid(), nombre:””, marca:””, producto:””, gramos:100,
calorias:0, proteinas:0, grasas:0, carbohidratos:0, azucares:0, fibra:0,
solidosPct:0, pod:0, pac:0, alergenos:[], notas:””,
buscado:false, buscando:false, errorBusqueda:null,
});

const emptyReceta = () => ({
id:uid(), nombre:“Nueva receta”, categoria:“Crema”, notas:””,
ingredientes:[emptyIng()],
creadaEn:new Date().toISOString(), modificadaEn:new Date().toISOString(),
});

function calcFormula(ings) {
const total = ings.reduce((s,i)=>s+(Number(i.gramos)||0),0);
if (!total) return {total:0,pod:0,pac:0,solidos:0,calorias:0,proteinas:0,grasas:0,carbohidratos:0,azucares:0,fibra:0};
const wAvg = f => ings.reduce((s,i)=>s+(Number(i[f])||0)*(Number(i.gramos)||0)/100,0)/total*100;
const podT = ings.reduce((s,i)=>s+(Number(i.pod)||0)*(Number(i.azucares)||0)*(Number(i.gramos)||0)/100,0);
const pacT = ings.reduce((s,i)=>s+(Number(i.pac)||0)*(Number(i.azucares)||0)*(Number(i.gramos)||0)/100,0);
const solT = ings.reduce((s,i)=>s+(Number(i.solidosPct)||0)*(Number(i.gramos)||0)/100,0);
return {
total, pod:(podT/total)*100, pac:(pacT/total)*100, solidos:(solT/total)*100,
calorias:wAvg(“calorias”), proteinas:wAvg(“proteinas”), grasas:wAvg(“grasas”),
carbohidratos:wAvg(“carbohidratos”), azucares:wAvg(“azucares”), fibra:wAvg(“fibra”),
};
}

async function buscarNutricion(marca, producto) {
const prompt = `Eres un experto en formulación de helados artesanales. El usuario trabaja con '${producto}' de la marca '${marca}'. Devuelve SOLO JSON válido (sin markdown, sin texto extra): {'nombre':'nombre corto','calorias':número,'proteinas':número,'grasas':número,'carbohidratos':número,'azucares':número,'fibra':número,'solidosPct':número,'pod':número,'pac':número,'alergenos':[],'notas':'tip breve'} Valores por 100g. solidosPct 0-100, pod 0-2 (sacarosa=1.0), pac 0-3 (sacarosa=1.0).`;
const resp = await fetch(“https://api.anthropic.com/v1/messages”,{
method:“POST”, headers:{“Content-Type”:“application/json”},
body:JSON.stringify({model:“claude-sonnet-4-20250514”,max_tokens:800,messages:[{role:“user”,content:prompt}]}),
});
const data = await resp.json();
const text = data.content?.find(b=>b.type===“text”)?.text||””;
return JSON.parse(text.replace(/`json|`/g,””).trim());
}

function Gauge({ valor, min, max, label }) {
const scale = 55;
const pct   = Math.min(100,Math.max(0,(valor/scale)*100));
const rMin  = (min/scale)*100;
const rMax  = (max/scale)*100;
const ok    = valor>=min && valor<=max;
const color = ok ? C.celesteDark : C.duraznoDeep;
return (
<div style={{marginBottom:16}}>
<div style={{display:“flex”,justifyContent:“space-between”,alignItems:“center”,marginBottom:6}}>
<span style={{fontSize:11,color:C.inkLight,fontWeight:500}}>{label}</span>
<div style={{display:“flex”,alignItems:“center”,gap:6}}>
<span style={{fontSize:17,fontWeight:800,color,fontVariantNumeric:“tabular-nums”}}>{valor.toFixed(1)}%</span>
<span style={{fontSize:9,padding:“2px 8px”,borderRadius:20,fontWeight:700,
background:ok?C.celesteLight:”#f7e8d8”,color}}>
{ok?“✓ ok”:valor<min?“↓ bajo”:“↑ alto”}
</span>
</div>
</div>
<div style={{position:“relative”,height:7,background:C.crema,borderRadius:4,overflow:“hidden”}}>
<div style={{position:“absolute”,left:`${rMin}%`,width:`${rMax-rMin}%`,height:“100%”,
background:`${C.celeste}50`}}/>
<div style={{position:“absolute”,left:0,width:`${pct}%`,height:“100%”,borderRadius:4,
background:ok?`linear-gradient(90deg,${C.celeste},${C.celesteDark})`:`linear-gradient(90deg,${C.durazno},${C.duraznoDeep})`,
transition:“width .4s ease”}}/>
</div>
<div style={{display:“flex”,justifyContent:“space-between”,marginTop:3}}>
<span style={{fontSize:9,color:C.inkFaint}}>{min}% mín</span>
<span style={{fontSize:9,color:C.inkFaint}}>{max}% máx</span>
</div>
</div>
);
}

function Tag({ label, active, onClick, warn }) {
return (
<button onClick={onClick} style={{
padding:“4px 11px”,borderRadius:20,fontSize:10,cursor:“pointer”,fontWeight:active?700:400,
border:`1.5px solid ${active?(warn?C.duraznoDeep:C.celesteDark):(warn?'#f0d0b0':C.cremaDeep)}`,
background:active?(warn?”#fff5e6”:C.celesteLight):“transparent”,
color:active?(warn?C.duraznoDeep:C.celesteDark):C.inkLight,
transition:“all .15s”,
}}>{label}</button>
);
}

function IngCard({ ing, onChange, onDelete }) {
const [open, setOpen] = useState(false);
const [marcaIn, setMarcaIn] = useState(ing.marca);
const [prodIn, setProdIn]   = useState(ing.producto);

const buscar = async () => {
if (!marcaIn.trim()||!prodIn.trim()) return;
onChange({…ing,marca:marcaIn,producto:prodIn,buscando:true,errorBusqueda:null});
try {
const d = await buscarNutricion(marcaIn,prodIn);
onChange({…ing,marca:marcaIn,producto:prodIn,nombre:d.nombre||prodIn,
calorias:d.calorias||0,proteinas:d.proteinas||0,grasas:d.grasas||0,
carbohidratos:d.carbohidratos||0,azucares:d.azucares||0,fibra:d.fibra||0,
solidosPct:d.solidosPct||0,pod:d.pod||0,pac:d.pac||0,
alergenos:d.alergenos||[],notas:d.notas||””,buscado:true,buscando:false,errorBusqueda:null});
setOpen(false);
} catch {
onChange({…ing,marca:marcaIn,producto:prodIn,buscando:false,
errorBusqueda:“No se encontraron datos. Ingresalos manualmente.”});
}
};

const numField = (key,lbl,step=0.1) => (
<div style={{flex:1,minWidth:68}}>
<div style={{fontSize:9,color:C.inkFaint,marginBottom:3,fontWeight:700,
textTransform:“uppercase”,letterSpacing:.8}}>{lbl}</div>
<input type=“number” step={step} value={ing[key]}
onChange={e=>onChange({…ing,[key]:parseFloat(e.target.value)||0})}
style={{width:“100%”,background:C.crema,border:`1.5px solid ${C.cremaDeep}`,
borderRadius:8,color:C.ink,padding:“6px 8px”,fontSize:12,fontWeight:600,
boxSizing:“border-box”,outline:“none”}}/>
</div>
);

const inputStyle = {
background:C.white,border:`1.5px solid ${C.cremaDeep}`,
borderRadius:8,color:C.ink,padding:“7px 10px”,fontSize:12,outline:“none”,
};

return (
<div style={{background:C.white,
border:`1.5px solid ${ing.buscado?C.celesteLight:C.cremaDeep}`,
borderRadius:14,marginBottom:8,overflow:“hidden”,
boxShadow:ing.buscado?“0 2px 10px rgba(166,215,217,.2)”:“0 1px 4px rgba(0,0,0,.04)”}}>

```
  <div style={{display:'flex',alignItems:'center',padding:'12px 14px',gap:10,cursor:'pointer'}}
    onClick={()=>setOpen(o=>!o)}>
    <div style={{width:38,height:38,borderRadius:10,flexShrink:0,
      background:ing.buscado?C.celesteLight:C.crema,
      display:'flex',alignItems:'center',justifyContent:'center',fontSize:18}}>
      {ing.buscado?'🧪':'⊕'}
    </div>
    <div style={{flex:1,minWidth:0}}>
      {ing.buscado
        ? <>
            <div style={{fontWeight:700,fontSize:13,color:C.ink,
              whiteSpace:'nowrap',overflow:'hidden',textOverflow:'ellipsis'}}>{ing.nombre}</div>
            <div style={{fontSize:10,color:C.inkFaint,marginTop:1}}>{ing.marca}{ing.producto?` · ${ing.producto}`:''}</div>
          </>
        : <div style={{fontSize:12,color:C.inkFaint,fontStyle:'italic'}}>Tocá para identificar…</div>
      }
    </div>
    <div style={{display:'flex',alignItems:'center',gap:4}} onClick={e=>e.stopPropagation()}>
      <input type='number' value={ing.gramos} min={0} step={5}
        onChange={e=>onChange({...ing,gramos:parseFloat(e.target.value)||0})}
        style={{width:58,background:C.crema,border:`1.5px solid ${C.cremaDeep}`,
          borderRadius:8,color:C.ink,padding:'5px 7px',fontSize:13,
          fontWeight:800,textAlign:'right',outline:'none'}}/>
      <span style={{fontSize:10,color:C.inkFaint,fontWeight:600}}>g</span>
    </div>
    <button onClick={e=>{e.stopPropagation();onDelete();}}
      style={{background:'none',border:'none',color:C.inkFaint,
        cursor:'pointer',fontSize:20,lineHeight:1,padding:'0 2px',opacity:.4}}>×</button>
    <span style={{fontSize:11,color:C.inkFaint,
      display:'inline-block',transform:open?'rotate(180deg)':'rotate(0)',transition:'transform .2s'}}>▾</span>
  </div>

  {open && (
    <div style={{padding:'0 14px 14px',borderTop:`1px solid ${C.crema}`}}>

      <div style={{marginTop:12,marginBottom:14,background:C.crema,borderRadius:12,padding:14}}>
        <div style={{fontSize:10,color:C.celesteDark,fontWeight:800,
          textTransform:'uppercase',letterSpacing:1,marginBottom:9}}>🔍 Identificar con IA</div>
        <div style={{display:'flex',gap:6,marginBottom:8}}>
          <input placeholder='Marca' value={marcaIn} onChange={e=>setMarcaIn(e.target.value)}
            style={{...inputStyle,flex:1}}/>
          <input placeholder='Producto / descripción' value={prodIn}
            onChange={e=>setProdIn(e.target.value)}
            onKeyDown={e=>e.key==='Enter'&&buscar()}
            style={{...inputStyle,flex:2}}/>
        </div>
        <button onClick={buscar} disabled={ing.buscando||!marcaIn.trim()||!prodIn.trim()}
          style={{width:'100%',padding:'9px',border:'none',borderRadius:9,
            fontWeight:700,fontSize:12,cursor:ing.buscando?'default':'pointer',letterSpacing:.3,
            background:ing.buscando?C.cremaDeep:`linear-gradient(135deg,${C.celeste},${C.celesteDark})`,
            color:ing.buscando?C.inkFaint:C.white,transition:'all .2s'}}>
          {ing.buscando?'Buscando valores…':'Buscar valores nutricionales'}
        </button>
        {ing.errorBusqueda&&<div style={{fontSize:11,color:C.danger,marginTop:6}}>{ing.errorBusqueda}</div>}
      </div>

      <div style={{fontSize:10,color:C.inkFaint,fontWeight:700,textTransform:'uppercase',letterSpacing:.8,marginBottom:8}}>
        Valores por 100 g
      </div>
      <div style={{display:'flex',flexWrap:'wrap',gap:7,marginBottom:14}}>
        {numField('calorias','kcal',1)}
        {numField('proteinas','Proteína')}
        {numField('grasas','Grasas')}
        {numField('carbohidratos','Carbos')}
        {numField('azucares','Azúcar')}
        {numField('fibra','Fibra')}
      </div>

      <div style={{fontSize:10,color:C.inkFaint,fontWeight:700,textTransform:'uppercase',letterSpacing:.8,marginBottom:8}}>
        Parámetros técnicos
      </div>
      <div style={{display:'flex',gap:7,marginBottom:14}}>
        {numField('solidosPct','Sólidos %',1)}
        {numField('pod','POD',.01)}
        {numField('pac','PAC',.01)}
      </div>

      <div style={{fontSize:10,color:C.inkFaint,fontWeight:700,textTransform:'uppercase',letterSpacing:.8,marginBottom:8}}>
        Alérgenos
      </div>
      <div style={{display:'flex',flexWrap:'wrap',gap:5}}>
        {Object.entries(ALERGENOS_ES).map(([k,v])=>{
          const on=ing.alergenos.includes(k);
          return <Tag key={k} label={v} active={on} warn
            onClick={()=>onChange({...ing,alergenos:on?ing.alergenos.filter(a=>a!==k):[...ing.alergenos,k]})}/>;
        })}
      </div>

      {ing.notas&&(
        <div style={{marginTop:10,padding:'9px 12px',background:C.celesteLight,
          borderRadius:10,fontSize:11,color:C.celesteDark,lineHeight:1.6}}>
          💡 {ing.notas}
        </div>
      )}
    </div>
  )}
</div>
```

);
}

function RecetaCard({ receta, activa, onSelect, onDelete }) {
const stats = calcFormula(receta.ingredientes);
const ok=(v,mn,mx)=>v>=mn&&v<=mx;
const dot=(v,mn,mx)=>(
<div style={{width:8,height:8,borderRadius:“50%”,
background:ok(v,mn,mx)?C.celesteDark:C.duraznoDeep}}/>
);
return (
<div onClick={onSelect} style={{
padding:“12px 16px”,cursor:“pointer”,
background:activa?C.celesteLight:C.white,
borderBottom:`1px solid ${C.crema}`,
display:“flex”,alignItems:“center”,gap:12,
transition:“background .15s”,
}}>
{activa&&<div style={{width:3,height:38,background:C.celesteDark,borderRadius:3,flexShrink:0}}/>}
<div style={{fontSize:24,flexShrink:0}}>{CAT_EMOJI[receta.categoria]||“🍨”}</div>
<div style={{flex:1,minWidth:0}}>
<div style={{fontWeight:700,fontSize:13,color:C.ink,
whiteSpace:“nowrap”,overflow:“hidden”,textOverflow:“ellipsis”}}>{receta.nombre}</div>
<div style={{fontSize:10,color:C.inkFaint,marginTop:1}}>
{receta.categoria} · {receta.ingredientes.length} ing · {stats.total.toLocaleString()} g
</div>
</div>
<div style={{display:“flex”,gap:5,alignItems:“center”}}>
{dot(stats.pod,22,26)}{dot(stats.pac,22,28)}{dot(stats.solidos,38,46)}
</div>
<button onClick={e=>{e.stopPropagation();onDelete();}}
style={{background:“none”,border:“none”,color:C.inkFaint,cursor:“pointer”,
fontSize:18,padding:“0 2px”,flexShrink:0,opacity:.35}}>×</button>
</div>
);
}

export default function GelatoLab() {
const [recetas, setRecetas] = useState(()=>[emptyReceta()]);
const [activeId, setActiveId] = useState(()=>recetas[0]?.id);
const [tab, setTab] = useState(“formula”);
const [vista, setVista] = useState(“editor”);
const [toast, setToast] = useState(””);

const receta = recetas.find(r=>r.id===activeId)||recetas[0];
const showToast = msg=>{setToast(msg);setTimeout(()=>setToast(””),2200);};

const updReceta = r=>setRecetas(p=>p.map(x=>x.id===r.id?{…r,modificadaEn:new Date().toISOString()}:x));
const addReceta = ()=>{const r=emptyReceta();setRecetas(p=>[r,…p]);setActiveId(r.id);setVista(“editor”);};
const delReceta = id=>{
const next=recetas.filter(r=>r.id!==id);
setRecetas(next.length?next:[emptyReceta()]);
if(activeId===id) setActiveId(next[0]?.id||null);
};
const dupReceta = ()=>{
const r={…receta,id:uid(),nombre:receta.nombre+” (copia)”,
ingredientes:receta.ingredientes.map(i=>({…i,id:uid()})),
creadaEn:new Date().toISOString(),modificadaEn:new Date().toISOString()};
setRecetas(p=>[r,…p]);setActiveId(r.id);showToast(“Receta duplicada ✓”);
};
const addIng = ()=>updReceta({…receta,ingredientes:[…receta.ingredientes,emptyIng()]});
const delIng = id=>updReceta({…receta,ingredientes:receta.ingredientes.filter(i=>i.id!==id)});
const updIng = ing=>updReceta({…receta,ingredientes:receta.ingredientes.map(i=>i.id===ing.id?ing:i)});

const exportar = ()=>{
const blob=new Blob([JSON.stringify(recetas,null,2)],{type:“application/json”});
const url=URL.createObjectURL(blob);
const a=document.createElement(“a”);a.href=url;a.download=“gelatolab_recetas.json”;a.click();
URL.revokeObjectURL(url);showToast(“Exportado ✓”);
};
const importar = e=>{
const file=e.target.files[0];if(!file)return;
const reader=new FileReader();
reader.onload=ev=>{
try{
const data=JSON.parse(ev.target.result);
if(Array.isArray(data)&&data.length){setRecetas(data);setActiveId(data[0].id);showToast(“Importado ✓”);}
}catch{showToast(“Error al importar”);}
};
reader.readAsText(file);e.target.value=””;
};

const stats=calcFormula(receta?.ingredientes||[]);
const alergenosFormula=[…new Set((receta?.ingredientes||[]).flatMap(i=>i.alergenos))];

const TabBtn=({id,label,emoji})=>(
<button onClick={()=>setTab(id)} style={{
flex:1,padding:“11px 4px”,background:“none”,border:“none”,
borderBottom:`2.5px solid ${tab===id?C.celesteDark:'transparent'}`,
color:tab===id?C.celesteDark:C.inkFaint,
fontSize:11,fontWeight:tab===id?800:500,cursor:“pointer”,transition:“all .2s”,
}}>{emoji} {label}</button>
);

if(!receta) return null;

return (
<div style={{minHeight:“100vh”,background:C.crema,color:C.ink,
fontFamily:”‘Plus Jakarta Sans’,system-ui,sans-serif”,paddingBottom:90}}>
<link href='https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&display=swap' rel='stylesheet'/>

```
  {/* Toast */}
  {toast&&(
    <div style={{position:'fixed',top:16,left:'50%',transform:'translateX(-50%)',
      background:C.celesteDark,color:C.white,padding:'8px 20px',borderRadius:20,
      fontSize:12,fontWeight:700,zIndex:9999,boxShadow:'0 4px 16px rgba(0,0,0,.15)',
      whiteSpace:'nowrap'}}>
      {toast}
    </div>
  )}

  {/* HEADER */}
  <div style={{background:C.white,padding:'16px 16px 12px',
    borderBottom:`1px solid ${C.crema}`,boxShadow:'0 1px 8px rgba(0,0,0,.06)'}}>
    <div style={{display:'flex',alignItems:'center',justifyContent:'space-between',
      marginBottom:vista==='editor'?10:0}}>
      <div style={{display:'flex',alignItems:'center',gap:9}}>
        <div style={{width:36,height:36,borderRadius:11,background:C.celeste,
          display:'flex',alignItems:'center',justifyContent:'center',fontSize:18,
          boxShadow:`0 2px 8px ${C.celeste}80`}}>🧊</div>
        <div>
          <div style={{fontSize:14,fontWeight:800,color:C.ink,letterSpacing:-.4,lineHeight:1.1}}>GELATOlab</div>
          <div style={{fontSize:8,color:C.inkFaint,letterSpacing:2,textTransform:'uppercase',lineHeight:1.2}}>EnergyLab.cc</div>
        </div>
      </div>
      <div style={{display:'flex',gap:5,alignItems:'center'}}>
        <button onClick={exportar}
          style={{background:C.crema,border:`1px solid ${C.cremaDeep}`,borderRadius:8,
            color:C.inkLight,fontSize:10,fontWeight:600,padding:'5px 10px',cursor:'pointer'}}>
          ↓ Export
        </button>
        <label style={{background:C.crema,border:`1px solid ${C.cremaDeep}`,borderRadius:8,
          color:C.inkLight,fontSize:10,fontWeight:600,padding:'5px 10px',cursor:'pointer'}}>
          ↑ Import<input type='file' accept='.json' onChange={importar} style={{display:'none'}}/>
        </label>
        <button onClick={()=>setVista(v=>v==='lista'?'editor':'lista')}
          style={{background:vista==='lista'?C.celeste:C.crema,
            border:`1px solid ${vista==='lista'?C.celesteDark:C.cremaDeep}`,
            borderRadius:8,color:vista==='lista'?C.celesteDark:C.inkLight,
            fontSize:10,fontWeight:700,padding:'5px 10px',cursor:'pointer'}}>
          {vista==='lista'?'✕ Cerrar':'≡ Recetas'}
        </button>
      </div>
    </div>

    {vista==='editor'&&(
      <>
        <input value={receta.nombre} onChange={e=>updReceta({...receta,nombre:e.target.value})}
          style={{background:'none',border:'none',borderBottom:`2px solid ${C.cremaDeep}`,
            color:C.ink,fontSize:20,fontWeight:800,width:'100%',outline:'none',
            paddingBottom:4,letterSpacing:-.5,marginBottom:10}}/>
        <div style={{display:'flex',gap:5,alignItems:'center',flexWrap:'wrap'}}>
          {CATEGORIAS_RECETA.map(c=>(
            <Tag key={c} label={`${CAT_EMOJI[c]} ${c}`} active={receta.categoria===c}
              onClick={()=>updReceta({...receta,categoria:c})}/>
          ))}
          <span style={{marginLeft:'auto',fontSize:10,color:C.inkFaint,fontWeight:500}}>
            {receta.ingredientes.length} ing · {stats.total.toLocaleString()} g
          </span>
          <button onClick={dupReceta}
            style={{background:'none',border:`1px solid ${C.cremaDeep}`,borderRadius:7,
              color:C.inkLight,fontSize:10,fontWeight:600,padding:'3px 9px',cursor:'pointer'}}>
            ⧉ Duplicar
          </button>
        </div>
      </>
    )}
  </div>

  {/* LISTA */}
  {vista==='lista'&&(
    <div style={{background:C.white,minHeight:'calc(100vh - 70px)'}}>
      <div style={{padding:'12px 16px',borderBottom:`1px solid ${C.crema}`,
        display:'flex',justifyContent:'space-between',alignItems:'center'}}>
        <span style={{fontSize:11,color:C.inkFaint,fontWeight:600}}>
          {recetas.length} {recetas.length===1?'receta':'recetas'} · <span style={{color:C.inkFaint}}>POD PAC SOL</span>
        </span>
        <button onClick={addReceta}
          style={{background:`linear-gradient(135deg,${C.celeste},${C.celesteDark})`,
            border:'none',borderRadius:9,color:C.white,fontSize:12,fontWeight:800,
            padding:'7px 14px',cursor:'pointer',boxShadow:`0 2px 8px ${C.celeste}80`}}>
          + Nueva receta
        </button>
      </div>
      {recetas.map(r=>(
        <RecetaCard key={r.id} receta={r} activa={r.id===activeId}
          onSelect={()=>{setActiveId(r.id);setVista('editor');setTab('formula');}}
          onDelete={()=>delReceta(r.id)}/>
      ))}
    </div>
  )}

  {/* EDITOR */}
  {vista==='editor'&&(
    <>
      <div style={{display:'flex',background:C.white,borderBottom:`1px solid ${C.crema}`}}>
        <TabBtn id='formula'   label='Fórmula'   emoji='⚗️'/>
        <TabBtn id='nutricion' label='Nutrición'  emoji='📊'/>
        <TabBtn id='alergenos' label='Alérgenos'  emoji='⚠️'/>
      </div>

      <div style={{padding:'16px 14px',maxWidth:520,margin:'0 auto'}}>

        {tab==='formula'&&(
          <>
            <div style={{background:C.white,borderRadius:16,padding:18,marginBottom:14,
              boxShadow:'0 2px 12px rgba(0,0,0,.06)'}}>
              <div style={{fontSize:10,fontWeight:800,color:C.inkFaint,
                textTransform:'uppercase',letterSpacing:1,marginBottom:16}}>Balance técnico</div>
              <Gauge valor={stats.pod}     min={22} max={26} label='POD — Dulzor'/>
              <Gauge valor={stats.pac}     min={22} max={28} label='PAC — Anticongelante'/>
              <Gauge valor={stats.solidos} min={38} max={46} label='Sólidos totales'/>
            </div>

            <div style={{fontSize:10,fontWeight:800,color:C.inkFaint,
              textTransform:'uppercase',letterSpacing:1,marginBottom:10}}>Ingredientes</div>
            {receta.ingredientes.map(ing=>(
              <IngCard key={ing.id} ing={ing} onChange={updIng} onDelete={()=>delIng(ing.id)}/>
            ))}
            <button onClick={addIng}
              style={{width:'100%',marginTop:6,padding:'13px',
                background:C.white,border:`2px dashed ${C.cremaDeep}`,
                borderRadius:12,color:C.inkFaint,fontSize:12,fontWeight:600,
                cursor:'pointer',transition:'all .2s'}}>
              + Agregar ingrediente
            </button>
          </>
        )}

        {tab==='nutricion'&&(
          <>
            <div style={{background:C.white,borderRadius:16,overflow:'hidden',
              marginBottom:14,boxShadow:'0 2px 12px rgba(0,0,0,.06)'}}>
              <div style={{padding:'14px 16px',borderBottom:`1px solid ${C.crema}`,
                display:'flex',justifyContent:'space-between',alignItems:'baseline'}}>
                <span style={{fontWeight:800,fontSize:15,color:C.ink}}>Información nutricional</span>
                <span style={{fontSize:10,color:C.inkFaint,fontWeight:600}}>por 100 g</span>
              </div>
              {[
                ['Valor energético',stats.calorias.toFixed(0),'kcal',false],
                ['Proteínas',stats.proteinas.toFixed(1),'g',false],
                ['Grasas totales',stats.grasas.toFixed(1),'g',false],
                ['Hidratos de carbono',stats.carbohidratos.toFixed(1),'g',false],
                ['  de los cuales azúcares',stats.azucares.toFixed(1),'g',true],
                ['Fibra alimentaria',stats.fibra.toFixed(1),'g',false],
              ].map(([lbl,val,u,sub],i)=>(
                <div key={i} style={{display:'flex',justifyContent:'space-between',alignItems:'center',
                  padding:'10px 16px',borderBottom:`1px solid ${C.crema}`,
                  background:i%2?C.crema:'transparent'}}>
                  <span style={{fontSize:12,color:sub?C.inkFaint:C.ink,
                    paddingLeft:sub?14:0,fontWeight:sub?400:500}}>{lbl.trim()}</span>
                  <span style={{fontWeight:700,fontSize:13,color:C.ink}}>
                    {val} <span style={{fontWeight:400,fontSize:11,color:C.inkFaint}}>{u}</span>
                  </span>
                </div>
              ))}
            </div>

            <div style={{background:C.white,borderRadius:16,padding:16,
              boxShadow:'0 2px 12px rgba(0,0,0,.06)'}}>
              <div style={{fontSize:10,fontWeight:800,color:C.inkFaint,
                textTransform:'uppercase',letterSpacing:1,marginBottom:14}}>Distribución de macros</div>
              {[
                {l:'Grasas',v:stats.grasas*9,c:C.duraznoDeep,bg:'#f7e8d8'},
                {l:'Carbohidratos',v:stats.carbohidratos*4,c:C.celesteDark,bg:C.celesteLight},
                {l:'Proteínas',v:stats.proteinas*4,c:C.olivaDark,bg:'#f0f0c0'},
              ].map(({l,v,c,bg})=>{
                const tot=(stats.grasas*9)+(stats.carbohidratos*4)+(stats.proteinas*4)||1;
                const p=(v/tot)*100;
                return (
                  <div key={l} style={{marginBottom:12}}>
                    <div style={{display:'flex',justifyContent:'space-between',alignItems:'center',marginBottom:6}}>
                      <span style={{fontSize:12,fontWeight:500,color:C.inkLight}}>{l}</span>
                      <span style={{fontSize:12,fontWeight:800,color:c,
                        background:bg,padding:'2px 9px',borderRadius:20}}>{p.toFixed(0)}%</span>
                    </div>
                    <div style={{height:6,background:C.crema,borderRadius:4}}>
                      <div style={{width:`${p}%`,height:'100%',background:c,borderRadius:4,transition:'width .5s'}}/>
                    </div>
                  </div>
                );
              })}
            </div>
          </>
        )}

        {tab==='alergenos'&&(
          <>
            <div style={{background:C.white,borderRadius:16,padding:16,marginBottom:14,
              boxShadow:'0 2px 12px rgba(0,0,0,.06)'}}>
              <div style={{fontSize:10,fontWeight:800,color:C.inkFaint,
                textTransform:'uppercase',letterSpacing:1,marginBottom:12}}>Alérgenos presentes</div>
              {alergenosFormula.length===0
                ? <div style={{color:C.inkFaint,fontSize:12,lineHeight:1.6}}>
                    Sin alérgenos detectados. Identificá todos los ingredientes para asegurarte.
                  </div>
                : <div style={{display:'flex',flexWrap:'wrap',gap:7}}>
                    {alergenosFormula.map(a=>(
                      <div key={a} style={{padding:'6px 14px',borderRadius:20,
                        background:'#fff5e6',border:`1.5px solid ${C.duraznoDeep}`,
                        color:C.duraznoDeep,fontSize:12,fontWeight:700}}>
                        ⚠️ {ALERGENOS_ES[a]}
                      </div>
                    ))}
                  </div>
              }
            </div>

            <div style={{background:C.white,borderRadius:16,overflow:'hidden',
              boxShadow:'0 2px 12px rgba(0,0,0,.06)'}}>
              <div style={{padding:'12px 16px',borderBottom:`1px solid ${C.crema}`,
                fontSize:10,fontWeight:800,color:C.inkFaint,textTransform:'uppercase',letterSpacing:1}}>
                Detalle por ingrediente
              </div>
              {receta.ingredientes.map(ing=>(
                <div key={ing.id} style={{padding:'10px 16px',borderBottom:`1px solid ${C.crema}`,
                  display:'flex',justifyContent:'space-between',alignItems:'center'}}>
                  <div>
                    <div style={{fontSize:12,fontWeight:700,color:C.ink}}>
                      {ing.nombre||ing.producto||'Sin identificar'}
                    </div>
                    <div style={{fontSize:10,color:C.inkFaint}}>{ing.gramos} g</div>
                  </div>
                  <div style={{display:'flex',flexWrap:'wrap',gap:4,maxWidth:'55%',justifyContent:'flex-end'}}>
                    {ing.alergenos.length===0
                      ? <span style={{fontSize:10,color:C.cremaDeep}}>—</span>
                      : ing.alergenos.map(a=>(
                          <span key={a} style={{fontSize:9,background:'#fff5e6',
                            border:`1px solid ${C.durazno}`,color:C.duraznoDeep,
                            borderRadius:10,padding:'2px 7px',fontWeight:700}}>
                            {ALERGENOS_ES[a]}
                          </span>
                        ))
                    }
                  </div>
                </div>
              ))}
            </div>
          </>
        )}
      </div>
    </>
  )}

  {/* FAB */}
  {vista==='editor'&&(
    <button onClick={()=>setVista('lista')}
      style={{position:'fixed',bottom:22,right:16,
        background:`linear-gradient(135deg,${C.celeste},${C.celesteDark})`,
        border:'none',borderRadius:'50%',width:54,height:54,color:C.white,
        fontSize:22,cursor:'pointer',zIndex:100,
        boxShadow:`0 4px 20px rgba(73,143,145,.45)`,
        display:'flex',alignItems:'center',justifyContent:'center'}}>
      ≡
    </button>
  )}
</div>
```

);
}
