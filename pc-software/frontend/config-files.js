/* Portable profiles. Import never writes motor settings or arms the system. */
function validateProfile(payload, kind, fields) {
  if (!payload || payload.version !== 1 || payload.kind !== kind ||
      !payload.config || typeof payload.config !== "object" || Array.isArray(payload.config))
    throw new Error(ui("Formato ou tipo de arquivo incompatível.", "Unsupported file format or profile type."));
  const values = payload.config;
  if (Object.keys(values).some(k => !fields.some(f => f.n === k)))
    throw new Error(ui("O arquivo contém campos desconhecidos.", "The file contains unknown fields."));
  for (const f of fields) {
    const v = values[f.n];
    const valid = f.t === "bool" ? typeof v === "boolean" :
      typeof v === "number" && Number.isFinite(v) && v >= f.min && v <= f.max &&
      (f.t !== "int" || Number.isInteger(v));
    if (!valid) throw new Error(ui("Valor ausente ou inválido: ", "Missing or invalid value: ") + f.n);
  }
  if (kind === "motors" && values.brk_ov_end <= values.brk_ov_start)
    throw new Error(ui("O fim da rampa deve superar o início.", "Ramp end must exceed ramp start."));
  return values;
}

function downloadProfile(kind, config) {
  const blob = new Blob([JSON.stringify({version:1, kind, config}, null, 2) + "\n"], {type:"application/json"});
  const url = URL.createObjectURL(blob), a = document.createElement("a");
  a.href = url;
  a.download = `btow-${kind}-${new Date().toISOString().slice(0,10)}.json`;
  a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}

const beltProfileFields = BELT_ADV_CFG_KEYS.map(n => {
  const el = $("a_" + n);
  return {n, t:n === "pretension" ? "int" : "float", min:+el.min, max:+el.max};
}).concat(["invert_lat", "subtract_gravity", "auto_scale"].map(n => ({n, t:"bool"})));

function readProfile(kind) {
  const values = {};
  const fields = kind === "motors" ? CFG_FIELDS : beltProfileFields;
  fields.forEach(f => {
    const el = $(kind === "motors" ? "cfg_" + f.n : f.n === "auto_scale" ? "auto-g-scale" : "a_" + f.n);
    values[f.n] = f.t === "bool" ? el.checked : el.value.trim() === "" ? NaN : Number(el.value);
  });
  return validateProfile({version:1, kind, config:values}, kind, fields);
}

function profileStatus(kind, message, error=false) {
  if (kind === "motors") cfgSetStatus(message, error ? "err" : "ok");
  else { $("belt-file-status").textContent = message; $("belt-file-status").className = "axhint " + (error ? "err" : "ok"); }
}

for (const [prefix, kind] of [["motor", "motors"], ["belt", "belt"]]) {
  $(prefix + "-export").onclick = () => {
    try { downloadProfile(kind, readProfile(kind)); profileStatus(kind, ui("JSON exportado (valores exibidos).", "JSON exported (displayed values).")); }
    catch (e) { profileStatus(kind, e.message, true); }
  };
  $(prefix + "-import").onclick = () => $(prefix + "-import-file").click();
  $(prefix + "-import-file").onchange = async e => {
    const file = e.target.files[0];
    e.target.value = "";
    if (!file) return;
    try {
      if (file.size > 65536) throw new Error(ui("Arquivo muito grande (máximo 64 KB).", "File too large (64 KB maximum)."));
      const fields = kind === "motors" ? CFG_FIELDS : beltProfileFields;
      const values = validateProfile(JSON.parse(await file.text()), kind, fields);
      if (diagState.running) throw new Error(ui("Pare o diagnóstico antes de importar.", "Stop diagnostics before importing."));
      if (kind === "motors" && cfgState.pendingDump) throw new Error(ui("Aguarde a leitura da placa.", "Wait for the controller read to finish."));
      if (kind === "belt" && (!beltWs || beltWs.readyState !== 1))
        throw new Error(ui("Conecte ao servidor antes de importar.", "Connect to the server before importing."));
      if (!confirm(kind === "motors" ?
        ui("Carregar os ajustes nos campos? Revise antes de Aplicar + salvar. A calibração não será alterada.", "Load settings into the form? Review before Apply + save. Calibration will not change.") :
        ui("Substituir e salvar os ajustes do Belt Tensioner? O sistema será desarmado.", "Replace and save Belt Tensioner settings? The system will be disarmed."))) return;
      beltEmergencyStop();
      if (kind === "motors") {
        fields.forEach(f => {
          cfgWriteInput(f, values[f.n]);
          $("cfg_" + f.n).dispatchEvent(new Event("change"));
        });
        profileStatus(kind, ui("Importado para revisão; clique Aplicar + salvar para gravar. Calibração preservada.", "Imported for review; click Apply + save to write. Calibration preserved."));
      } else {
        clearTimeout(advancedCfgPending); advancedCfgPending = null; advancedCfgPersist = false;
        fields.forEach(f => {
          const el = $(f.n === "auto_scale" ? "auto-g-scale" : "a_" + f.n);
          if (f.t === "bool") el.checked = values[f.n];
          else { el.value = values[f.n]; advancedFormatOutput(f.n, values[f.n], $("v-a_" + f.n)); }
        });
        beltWsSend({type:"advanced_config", patch:values, persist:true});
        profileStatus(kind, ui("Importação enviada; confira o estado de salvamento automático. Requer armar manualmente.", "Import sent; check automatic-save status. Manual arming is required."));
      }
    } catch (err) { profileStatus(kind, err.message, true); }
  };
}
