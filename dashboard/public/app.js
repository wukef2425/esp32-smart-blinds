const $ = (id) => document.getElementById(id);
const number = (value, digits = 1) =>
  value === null || value === undefined ? "—" : Number(value).toFixed(digits);
const date = (value) => new Date(value).toLocaleString();
let busy = false;
function chart(key, points) {
  const host = $(key + "-chart");
  const valid = points.filter((p) => p[key] !== null);
  host.replaceChildren();
  if (!valid.length) {
    const p = document.createElement("p");
    p.className = "empty";
    p.textContent = "No valid measurements in this window";
    host.append(p);
    return;
  }
  const NS = "http://www.w3.org/2000/svg",
    svg = document.createElementNS(NS, "svg");
  svg.setAttribute("viewBox", "0 0 800 230");
  svg.setAttribute("role", "img");
  svg.setAttribute(
    "aria-label",
    `${key} history, ${valid.length} measurements`,
  );
  const make = (tag, attrs, text) => {
    const n = document.createElementNS(NS, tag);
    for (const [k, v] of Object.entries(attrs)) n.setAttribute(k, v);
    if (text !== undefined) n.textContent = text;
    svg.append(n);
    return n;
  };
  const times = points.map((p) => Date.parse(p.time)),
    minT = Math.min(...times),
    maxT = Math.max(...times);
  const values = valid.map((p) => p[key]),
    lo = Math.min(...values),
    hi = Math.max(...values),
    pad = Math.max((hi - lo) * 0.15, key === "pressure" ? 0.2 : 1),
    min = lo - pad,
    max = hi + pad;
  const x = (t) => 60 + ((t - minT) / Math.max(maxT - minT, 1)) * 720,
    y = (v) => 190 - ((v - min) / (max - min)) * 170;
  for (let i = 0; i < 4; i++) {
    const v = min + ((max - min) * i) / 3;
    make("line", { x1: 60, x2: 780, y1: y(v), y2: y(v), stroke: "#e1e9ed" });
    make("text", { x: 50, y: y(v) + 5, "text-anchor": "end" }, v.toFixed(1));
  }
  let path = "",
    previous = null;
  for (const p of points) {
    if (p[key] === null) {
      previous = null;
      continue;
    }
    const t = Date.parse(p.time);
    path +=
      (previous !== null && t - previous <= 600000 ? "L" : "M") +
      x(t) +
      " " +
      y(p[key]) +
      " ";
    previous = t;
  }
  make("path", {
    d: path,
    fill: "none",
    stroke: "#1b7997",
    "stroke-width": 2.5,
  });
  if (valid.length === 1)
    make("circle", {
      cx: x(Date.parse(valid[0].time)),
      cy: y(valid[0][key]),
      r: 4,
      fill: "#1b7997",
    });
  make(
    "text",
    { x: 60, y: 220 },
    new Date(minT).toLocaleString(undefined, {
      month: "short",
      day: "numeric",
      hour: "2-digit",
      minute: "2-digit",
    }),
  );
  make(
    "text",
    { x: 780, y: 220, "text-anchor": "end" },
    new Date(maxT).toLocaleString(undefined, {
      month: "short",
      day: "numeric",
      hour: "2-digit",
      minute: "2-digit",
    }),
  );
  host.append(svg);
}
async function load() {
  if (busy) return;
  busy = true;
  $("refresh").disabled = true;
  try {
    const response = await fetch("/api/data?range=" + $("range").value, {
        signal: AbortSignal.timeout(18000),
      }),
      data = await response.json();
    if (!response.ok) throw Error(data.error || "Unable to load telemetry");
    const p = data.latest;
    for (const key of ["temperature", "pressure", "rssi", "target"])
      $(key).textContent = number(
        p[key],
        ["rssi", "target"].includes(key) ? 0 : 1,
      );
    $("outdoor").textContent =
      p.outdoor === null ? "Unavailable" : number(p.outdoor) + " °C";
    $("uptime").textContent =
      p.uptime === null
        ? "Unavailable"
        : `${Math.floor(p.uptime / 3600)}h ${Math.floor((p.uptime % 3600) / 60)}m`;
    for (const key of ["boot", "sequence", "failed", "offline"])
      $(key).textContent = number(p[key], 0);
    const age = Date.now() - Date.parse(p.time),
      fresh = age >= -60000 && age < 900000;
    $("connection").textContent = fresh
      ? "Recent telemetry"
      : "Telemetry stale";
    $("connection").className = "badge " + (fresh ? "fresh" : "stale");
    $("updated").textContent = "Last device upload · " + date(p.time);
    for (const key of ["temperature", "pressure", "rssi"])
      chart(key, data.points);
    $("error").hidden = true;
  } catch (e) {
    $("error").textContent =
      e.name === "TimeoutError"
        ? "Request timed out. Retrying in one minute."
        : e.message;
    $("error").hidden = false;
    $("connection").textContent = "Connection error";
    $("connection").className = "badge stale";
  } finally {
    busy = false;
    $("refresh").disabled = false;
  }
}
$("refresh").addEventListener("click", load);
$("range").addEventListener("change", load);
setInterval(() => {
  if (!document.hidden) load();
}, 60000);
load();
