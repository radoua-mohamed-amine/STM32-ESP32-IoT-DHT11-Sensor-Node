/* ── Page navigation ────────────────────────────────────────────────── */
function showPage(id, el) {
  document
    .querySelectorAll(".page")
    .forEach((p) => p.classList.remove("active"));
  document
    .querySelectorAll(".tab")
    .forEach((t) => t.classList.remove("active"));
  document.getElementById("page-" + id).classList.add("active");
  el.classList.add("active");
}

/* ══════════════════════════════════════════════════════════════════════
 *  LIVE PAGE
 * ══════════════════════════════════════════════════════════════════════*/
let mqttClient = null;
let count = 0;
let tMin = Infinity,
  tMax = -Infinity;
let hMin = Infinity,
  hMax = -Infinity;
let tHist = [],
  hHist = [],
  tLabels = [];

/* CSV append state */
let csvHandle = null; /* FileSystemFileHandle */
let csvRows = []; /* in-memory buffer: [{ts,t,h}] */
let csvFileOpen = false;

const cOpts = {
  responsive: true,
  animation: { duration: 400 },
  plugins: { legend: { display: false } },
  scales: {
    y: {
      grid: { color: "rgba(255,255,255,0.04)" },
      ticks: {
        color: "#64748b",
        font: { size: 11, family: "JetBrains Mono" },
      },
    },
    x: {
      grid: { display: false },
      ticks: {
        color: "#64748b",
        font: { size: 10, family: "JetBrains Mono" },
        maxTicksLimit: 6,
      },
    },
  },
};

const liveTC = new Chart(document.getElementById("tc").getContext("2d"), {
  type: "line",
  data: {
    labels: tLabels,
    datasets: [
      {
        data: tHist,
        borderColor: "#f97316",
        backgroundColor: "rgba(249,115,22,0.08)",
        borderWidth: 2,
        pointRadius: 2,
        tension: 0.4,
        fill: true,
      },
    ],
  },
  options: {
    ...cOpts,
    scales: {
      ...cOpts.scales,
      y: { ...cOpts.scales.y, suggestedMin: 15, suggestedMax: 40 },
    },
  },
});
const liveHC = new Chart(document.getElementById("hc").getContext("2d"), {
  type: "line",
  data: {
    labels: tLabels,
    datasets: [
      {
        data: hHist,
        borderColor: "#3b82f6",
        backgroundColor: "rgba(59,130,246,0.08)",
        borderWidth: 2,
        pointRadius: 2,
        tension: 0.4,
        fill: true,
      },
    ],
  },
  options: {
    ...cOpts,
    scales: {
      ...cOpts.scales,
      y: { ...cOpts.scales.y, min: 0, max: 100 },
    },
  },
});

function gauge(id, val, mn, mx) {
  const off = 251 - Math.min(Math.max((val - mn) / (mx - mn), 0), 1) * 251;
  document
    .getElementById(id + "-arc")
    .setAttribute("stroke-dashoffset", off.toFixed(1));
  document.getElementById(id + "-gval").textContent = val.toFixed(1);
}

function mqttConnect() {
  if (mqttClient) mqttClient.end();
  const ip = document.getElementById("ip").value.trim();
  const port = document.getElementById("port").value.trim();
  const topic = document.getElementById("topic").value.trim();
  const url = `ws://${ip}:${port}/mqtt`;
  setStatus("Connecting...", null);
  mqttClient = mqtt.connect(url, {
    clientId: "stm32_dash_" + Math.random().toString(16).slice(2),
    reconnectPeriod: 3000,
  });

  mqttClient.on("connect", () => {
    setStatus("Connected  " + url, true);
    mqttClient.subscribe(topic);
    log(`<span style="color:var(--accent)">Connected → ${topic}</span>`);
  });

  mqttClient.on("message", (t, payload) => {
    try {
      const raw = payload.toString();
      const d = JSON.parse(raw);
      const temp = d.t !== undefined ? d.t : d.temperature;
      const hum = d.h !== undefined ? d.h : d.humidity;
      if (temp === undefined || hum === undefined) return;

      const now = new Date();
      const ts = now.toLocaleTimeString();
      count++;

      /* cards */
      document.getElementById("v-temp").textContent = temp.toFixed(1);
      document.getElementById("v-hum").textContent = hum.toFixed(0);
      document.getElementById("v-count").textContent = count;
      document.getElementById("v-last").textContent = ts;

      if (temp < tMin) tMin = temp;
      if (temp > tMax) tMax = temp;
      if (hum < hMin) hMin = hum;
      if (hum > hMax) hMax = hum;
      document.getElementById("t-min").textContent = tMin.toFixed(1);
      document.getElementById("t-max").textContent = tMax.toFixed(1);
      document.getElementById("h-min").textContent = hMin.toFixed(1);
      document.getElementById("h-max").textContent = hMax.toFixed(1);

      /* LEDs */
      document.getElementById("led-b").classList.toggle("active", temp < 20);
      document
        .getElementById("led-g")
        .classList.toggle("active", temp >= 20 && temp < 30);
      document.getElementById("led-r").classList.toggle("active", temp >= 30);

      /* gauges */
      gauge("t", temp, 0, 50);
      gauge("h", hum, 0, 100);

      /* charts */
      tLabels.push(ts);
      tHist.push(temp);
      hHist.push(hum);
      if (tLabels.length > 50) {
        tLabels.shift();
        tHist.shift();
        hHist.shift();
      }
      liveTC.update();
      liveHC.update();

      /* CSV buffer */
      csvRows.push({
        ts: now.toISOString(),
        t: temp.toFixed(2),
        h: hum.toFixed(0),
      });

      log(
        `<span style="color:var(--accent)">[${ts}]</span>  T: ${temp.toFixed(1)}°C  H: ${hum.toFixed(0)}%`,
      );
    } catch (e) {
      log(`<span style="color:var(--danger)">Parse error: ${e.message}</span>`);
    }
  });

  mqttClient.on("error", (e) => {
    setStatus("Error: " + e.message, false);
    log(`<span style="color:var(--danger)">${e.message}</span>`);
  });
  mqttClient.on("close", () => setStatus("Disconnected", false));
}

function mqttDisconnect() {
  if (mqttClient) {
    mqttClient.end();
    mqttClient = null;
  }
  setStatus("Disconnected", false);
}

function setStatus(msg, ok) {
  document.getElementById("dot").className =
    "dot " + (ok === true ? "on" : ok === false ? "off" : "");
  document.getElementById("status").textContent = msg;
}

function log(html) {
  const el = document.getElementById("log");
  el.innerHTML = html + "<br>" + el.innerHTML;
}

/* ── CSV Export (append to existing file via File System Access API) ── */
async function exportCSV() {
  if (csvRows.length === 0) {
    alert("No data yet — connect and wait for readings.");
    return;
  }

  const header = "timestamp,temperature_c,humidity_pct\n";
  const newRows = csvRows.map((r) => `${r.ts},${r.t},${r.h}`).join("\n") + "\n";

  /* Check if File System Access API is available */
  if (window.showSaveFilePicker) {
    try {
      if (!csvHandle) {
        /* First export: let user pick/create a file */
        csvHandle = await window.showSaveFilePicker({
          suggestedName: "stm32_esp32_data.csv",
          types: [
            { description: "CSV file", accept: { "text/csv": [".csv"] } },
          ],
        });
      }

      /* Read existing content */
      const existing = await csvHandle.getFile();
      const existingText = await existing.text();

      /* Append: if file already has data skip header, just add rows */
      const appendText = existingText.length > 0 ? newRows : header + newRows;

      const writable = await csvHandle.createWritable({
        keepExistingData: false,
      });
      await writable.write(
        existingText.length > 0 ? existingText + newRows : header + newRows,
      );
      await writable.close();

      log(
        `<span style="color:var(--accent)">CSV appended — ${csvRows.length} new rows saved</span>`,
      );
      csvRows = []; /* clear buffer after save */
    } catch (e) {
      if (e.name !== "AbortError")
        log(`<span style="color:var(--danger)">CSV error: ${e.message}</span>`);
    }
  } else {
    /* Fallback for browsers without File System Access API */
    const content =
      header + csvRows.map((r) => `${r.ts},${r.t},${r.h}`).join("\n");
    const blob = new Blob([content], { type: "text/csv" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download =
      "stm32_esp32_data_" +
      new Date().toISOString().slice(0, 19).replace(/:/g, "-") +
      ".csv";
    a.click();
    URL.revokeObjectURL(url);
    csvRows = [];
    log(
      `<span style="color:var(--accent)">CSV downloaded (${count} rows). Re-click to append next session.</span>`,
    );
  }
}

/* ══════════════════════════════════════════════════════════════════════
 *  IMPORT PAGE
 * ══════════════════════════════════════════════════════════════════════*/
let importedData = [];
let filteredData = [];
let importTChart = null;
let importHChart = null;
let currentPage = 1;
const ROWS_PER_PAGE = 20;

function handleDrop(e) {
  e.preventDefault();
  document.getElementById("drop-zone").style.borderColor = "";
  const file = e.dataTransfer.files[0];
  if (file) loadFile(file);
}

function loadFile(file) {
  if (!file) return;
  const reader = new FileReader();
  reader.onload = (e) => parseCSV(e.target.result);
  reader.readAsText(file);
}

function parseCSV(text) {
  const lines = text.trim().split("\n");
  if (lines.length < 2) {
    alert("CSV file is empty or invalid.");
    return;
  }

  /* detect separator */
  const sep = lines[0].includes(";") ? ";" : ",";
  const headers = lines[0].split(sep).map((h) => h.trim().toLowerCase());

  const ti = headers.findIndex((h) => h.includes("temp"));
  const hi = headers.findIndex((h) => h.includes("hum"));
  const tsi = headers.findIndex(
    (h) => h.includes("time") || h.includes("stamp") || h.includes("date"),
  );

  if (ti < 0 || hi < 0) {
    alert("Cannot find temperature or humidity columns.");
    return;
  }

  importedData = [];
  for (let i = 1; i < lines.length; i++) {
    const cols = lines[i].split(sep);
    if (cols.length < 2) continue;
    const t = parseFloat(cols[ti]);
    const h = parseFloat(cols[hi]);
    const ts = tsi >= 0 ? cols[tsi].trim() : "";
    if (isNaN(t) || isNaN(h)) continue;
    importedData.push({ ts, t, h, dt: ts ? new Date(ts) : null });
  }

  if (importedData.length === 0) {
    alert("No valid rows found.");
    return;
  }

  filteredData = [...importedData];
  renderImport();

  /* show filter row */
  const fr = document.getElementById("filter-row");
  fr.style.display = "flex";

  /* prefill datetime filters */
  const first = importedData.find((r) => r.dt);
  const last = [...importedData].reverse().find((r) => r.dt);
  if (first && last) {
    document.getElementById("f-from").value = toLocalInput(first.dt);
    document.getElementById("f-to").value = toLocalInput(last.dt);
  }
}

function toLocalInput(d) {
  const pad = (n) => String(n).padStart(2, "0");
  return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}T${pad(d.getHours())}:${pad(d.getMinutes())}`;
}

function applyFilter() {
  const from = document.getElementById("f-from").value;
  const to = document.getElementById("f-to").value;
  const fd = from ? new Date(from) : null;
  const td = to ? new Date(to) : null;

  filteredData = importedData.filter((r) => {
    if (!r.dt) return true;
    if (fd && r.dt < fd) return false;
    if (td && r.dt > td) return false;
    return true;
  });
  currentPage = 1;
  renderImport();
}

function clearFilter() {
  filteredData = [...importedData];
  currentPage = 1;
  renderImport();
  const first = importedData.find((r) => r.dt);
  const last = [...importedData].reverse().find((r) => r.dt);
  if (first) document.getElementById("f-from").value = toLocalInput(first.dt);
  if (last) document.getElementById("f-to").value = toLocalInput(last.dt);
}

function renderImport() {
  const d = filteredData;
  if (d.length === 0) return;

  /* show sections */
  document.getElementById("i-stats").style.display = "grid";
  document.getElementById("i-charts").style.display = "grid";
  document.getElementById("i-table-wrap").style.display = "block";
  document.getElementById("btn-export-filtered").style.display = "inline-block";

  /* stats */
  const temps = d.map((r) => r.t);
  const hums = d.map((r) => r.h);
  const avg = (arr) => (arr.reduce((a, b) => a + b, 0) / arr.length).toFixed(1);

  document.getElementById("i-rows").textContent = d.length;
  document.getElementById("i-tmin").textContent =
    Math.min(...temps).toFixed(1) + "°";
  document.getElementById("i-tmax").textContent =
    Math.max(...temps).toFixed(1) + "°";
  document.getElementById("i-tavg").textContent = avg(temps) + "°";
  document.getElementById("i-hmin").textContent =
    Math.min(...hums).toFixed(0) + "%";
  document.getElementById("i-hmax").textContent =
    Math.max(...hums).toFixed(0) + "%";
  document.getElementById("i-havg").textContent = avg(hums) + "%";

  /* duration */
  const withDt = d.filter((r) => r.dt);
  if (withDt.length >= 2) {
    const ms = withDt[withDt.length - 1].dt - withDt[0].dt;
    const min = Math.round(ms / 60000);
    document.getElementById("i-dur").textContent =
      min < 60 ? min + " min" : (min / 60).toFixed(1) + " h";
  }

  /* charts — subsample if too many points */
  const MAX_PTS = 100;
  const step = Math.max(1, Math.floor(d.length / MAX_PTS));
  const sub = d.filter((_, i) => i % step === 0);
  const lbl = sub.map((r) => (r.ts ? r.ts.slice(11, 16) : ""));
  const tv = sub.map((r) => r.t);
  const hv = sub.map((r) => r.h);

  if (importTChart) {
    importTChart.destroy();
    importHChart.destroy();
  }

  importTChart = new Chart(document.getElementById("i-tc").getContext("2d"), {
    type: "line",
    data: {
      labels: lbl,
      datasets: [
        {
          data: tv,
          borderColor: "#f97316",
          backgroundColor: "rgba(249,115,22,0.08)",
          borderWidth: 1.5,
          pointRadius: 0,
          tension: 0.3,
          fill: true,
        },
      ],
    },
    options: {
      ...cOpts,
      scales: {
        ...cOpts.scales,
        y: {
          ...cOpts.scales.y,
          suggestedMin: Math.min(...tv) - 2,
          suggestedMax: Math.max(...tv) + 2,
        },
      },
    },
  });
  importHChart = new Chart(document.getElementById("i-hc").getContext("2d"), {
    type: "line",
    data: {
      labels: lbl,
      datasets: [
        {
          data: hv,
          borderColor: "#3b82f6",
          backgroundColor: "rgba(59,130,246,0.08)",
          borderWidth: 1.5,
          pointRadius: 0,
          tension: 0.3,
          fill: true,
        },
      ],
    },
    options: {
      ...cOpts,
      scales: {
        ...cOpts.scales,
        y: { ...cOpts.scales.y, min: 0, max: 100 },
      },
    },
  });

  /* table */
  currentPage = 1;
  renderTable();
}

function renderTable() {
  const d = filteredData;
  const total = Math.ceil(d.length / ROWS_PER_PAGE);
  const start = (currentPage - 1) * ROWS_PER_PAGE;
  const slice = d.slice(start, start + ROWS_PER_PAGE);

  const tbody = document.getElementById("i-tbody");
  tbody.innerHTML = slice
    .map((r, i) => {
      const tClass = r.t >= 30 ? "hot" : r.t < 20 ? "cold" : "warm";
      const hClass = r.h > 80 ? "hot" : r.h < 30 ? "cold" : "";
      return `<tr>
      <td>${start + i + 1}</td>
      <td>${r.ts || "--"}</td>
      <td class="${tClass}">${r.t.toFixed(1)}</td>
      <td class="${hClass}">${r.h.toFixed(0)}</td>
    </tr>`;
    })
    .join("");

  document.getElementById("p-info").textContent =
    `page ${currentPage} / ${total}`;
  document.getElementById("p-prev").disabled = currentPage <= 1;
  document.getElementById("p-next").disabled = currentPage >= total;
}

function changePage(dir) {
  const total = Math.ceil(filteredData.length / ROWS_PER_PAGE);
  currentPage = Math.min(Math.max(currentPage + dir, 1), total);
  renderTable();
}

function exportFiltered() {
  if (filteredData.length === 0) return;
  const lines = [
    "timestamp,temperature_c,humidity_pct",
    ...filteredData.map((r) => `${r.ts},${r.t.toFixed(2)},${r.h.toFixed(0)}`),
  ];
  const blob = new Blob([lines.join("\n")], { type: "text/csv" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = "filtered_" + new Date().toISOString().slice(0, 10) + ".csv";
  a.click();
  URL.revokeObjectURL(url);
}
