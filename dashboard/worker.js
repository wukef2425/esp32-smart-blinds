const json = (data, status = 200) =>
  Response.json(data, {
    status,
    headers: {
      "Cache-Control": "no-store",
      "X-Content-Type-Options": "nosniff",
    },
  });
const numeric = (value) =>
  value === null ||
  value === undefined ||
  value === "" ||
  !Number.isFinite(Number(value))
    ? null
    : Number(value);
export function normalize(feed) {
  const status = Object.fromEntries(
    String(feed.status || "")
      .split(/\s+/)
      .map((s) => s.split("="))
      .filter((p) => p.length === 2),
  );
  const valid = status.indoor_valid !== "0";
  return {
    time: feed.created_at,
    id: numeric(feed.entry_id),
    temperature: valid ? numeric(feed.field1) : null,
    pressure: valid ? numeric(feed.field2) : null,
    rssi: numeric(feed.field3),
    target: numeric(status.target),
    outdoor: status.weather_valid === "1" ? numeric(status.outdoor) : null,
    uptime: numeric(status.uptime),
    boot: numeric(status.boot),
    sequence: numeric(status.seq),
    failed: numeric(status.failed),
    offline: numeric(status.offline),
  };
}
export default {
  async fetch(request, env, ctx) {
    const url = new URL(request.url);
    if (!url.pathname.startsWith("/api/")) return env.ASSETS.fetch(request);
    if (request.method !== "GET")
      return json({ error: "Method not allowed" }, 405);
    if (url.pathname !== "/api/data") return json({ error: "Not found" }, 404);
    const range = url.searchParams.get("range") || "24";
    if (!["6", "24", "72"].includes(range))
      return json({ error: "Invalid range" }, 400);
    if (
      !env.THINGSPEAK_READ_API_KEY ||
      !/^\d+$/.test(env.THINGSPEAK_CHANNEL_ID || "")
    )
      return json({ error: "Dashboard data source is not configured" }, 503);
    const cacheKey = new Request(
      `${url.origin}/_cache/data/${env.THINGSPEAK_CHANNEL_ID}/${range}`,
    );
    const cache = caches.default;
    const cached = await cache.match(cacheKey);
    if (cached) return cached;
    try {
      const upstream = async (path) => {
        const u = new URL(
          `https://api.thingspeak.com/channels/${env.THINGSPEAK_CHANNEL_ID}/${path}`,
        );
        u.searchParams.set("api_key", env.THINGSPEAK_READ_API_KEY);
        u.searchParams.set("status", "true");
        if (path === "feeds.json") {
          u.searchParams.set("results", "8000");
          u.searchParams.set(
            "start",
            new Date(Date.now() - Number(range) * 3600000).toISOString(),
          );
        }
        const r = await fetch(u, { signal: AbortSignal.timeout(12000) });
        if (!r.ok) throw Error("upstream");
        const data = await r.json();
        if (!data || typeof data !== "object") throw Error("format");
        return data;
      };
      const [history, latest] = await Promise.all([
        upstream("feeds.json"),
        upstream("feeds/last.json"),
      ]);
      if (!Array.isArray(history.feeds) || !latest.created_at)
        throw Error("format");
      const points = history.feeds
        .filter((f) => Number.isFinite(Date.parse(f.created_at)))
        .map(normalize);
      const current = normalize(latest);
      if (
        Date.parse(current.time) >= Date.now() - Number(range) * 3600000 &&
        !points.some((p) => p.id === current.id)
      )
        points.push(current);
      points.sort((a, b) => Date.parse(a.time) - Date.parse(b.time));
      const response = json({
        latest: current,
        points,
        fetchedAt: new Date().toISOString(),
        historyMayBeCached: true,
      });
      const stored = new Response(response.clone().body, response);
      stored.headers.set("Cache-Control", "public, max-age=60");
      ctx.waitUntil(cache.put(cacheKey, stored));
      return response;
    } catch {
      return json(
        {
          error:
            "Unable to read ThingSpeak. Check the channel and Read API Key, then retry.",
        },
        502,
      );
    }
  },
};
