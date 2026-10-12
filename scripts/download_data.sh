

set -euo pipefail

OUT_DIR="${1:-$HOME/Downloads/taxi2014}"
START="${START:-2014-01-01}"
END="${END:-2014-05-31}"
PAR="${PAR:-12}"
APP_TOKEN="${APP_TOKEN:-}"
URL="https://data.cityofnewyork.us/resource/gkne-dk5s.csv"

# next calendar day, on macOS (BSD date) or Linux (GNU date)
next_day() {
  date -j -v+1d -f %Y-%m-%d "$1" +%Y-%m-%d 2>/dev/null || date -d "$1 +1 day" +%Y-%m-%d
}

fetch_day() {
  local day="$1" next out
  next="$(next_day "$day")"
  out="$OUT_DIR/days/$day.csv"
  [ -f "$out" ] && return 0
  local hdr=()
  [ -n "$APP_TOKEN" ] && hdr=(-H "X-App-Token: $APP_TOKEN")
  if curl -sS -f --retry 5 --retry-delay 30 --speed-limit 1000 --speed-time 300 \
       ${hdr[@]+"${hdr[@]}"} -G "$URL" \
       --data-urlencode "\$where=pickup_datetime >= '${day}T00:00:00' AND pickup_datetime < '${next}T00:00:00'" \
       --data-urlencode "\$limit=5000000" \
       -o "$out.part"; then
    mv "$out.part" "$out"
    echo "$(date +%H:%M) $day done ($(( $(wc -l < "$out") - 1 )) trips)"
  else
    rm -f "$out.part"
    echo "$(date +%H:%M) $day FAILED (re-run the script to retry)" >&2
    return 1
  fi
}
export -f fetch_day next_day
export OUT_DIR URL APP_TOKEN

days=()
d="$START"
while [[ "$d" < "$END" || "$d" == "$END" ]]; do
  days+=("$d")
  d="$(next_day "$d")"
done

mkdir -p "$OUT_DIR/days"
echo "$(date +%H:%M) Downloading ${#days[@]} days ($START to $END) into $OUT_DIR/days, $PAR at a time"
start=$(date +%s)
printf '%s\n' "${days[@]}" | xargs -P "$PAR" -I{} bash -c 'fetch_day {}' || true

missing=0
for d in "${days[@]}"; do
  [ -f "$OUT_DIR/days/$d.csv" ] || missing=$((missing + 1))
done
if [ "$missing" -gt 0 ]; then
  echo "$missing days missing. Re-run the script to fetch them." >&2
  exit 1
fi
echo "All days downloaded in $(( ($(date +%s) - start) / 60 )) min"

FINAL="$OUT_DIR/yellow_tripdata_2014_jan_may.csv"
[ "$START" == "2014-01-01" ] && [ "$END" == "2014-05-31" ] || FINAL="$OUT_DIR/yellow_tripdata_${START}_to_${END}.csv"
head -n 1 "$OUT_DIR/days/${days[0]}.csv" > "$FINAL"
for d in "${days[@]}"; do
  tail -n +2 "$OUT_DIR/days/$d.csv"
done >> "$FINAL"

rows=$(( $(wc -l < "$FINAL") - 1 ))
echo "Done: $rows trips, $(du -h "$FINAL" | cut -f1) -> $FINAL"
echo "Expected for 2014-01-01 to 2014-05-31: 71667210 trips."
echo "Once the count matches, you can delete $OUT_DIR/days to free space."
