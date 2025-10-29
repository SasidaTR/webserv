#!/bin/bash
# ==========================================================
# Live concurrent test for your webserv (127.0.0.1:8080)
# Usage: ./multi_user_test.sh [users] [iterations]
# Example: ./multi_user_test.sh 5 10
# ==========================================================

USERS=${1:-5}          # concurrent users
ITER=${2:-5}           # requests per user
URL="http://127.0.0.1:8080/"

echo "Launching $USERS users × $ITER requests each on $URL"
echo "=========================================================="

COLORS=(31 32 33 34 35 36)

simulate_user() {
    local id=$1
    local color=${COLORS[$((id % ${#COLORS[@]}))]}

    for ((i=1; i<=ITER; i++)); do
        result=$(curl -s -o /dev/null -w "%{http_code} %{time_total}s" "$URL")
        code=$(echo "$result" | awk '{print $1}')
        time=$(echo "$result" | awk '{print $2}')
        printf "\033[%sm[User %02d]\033[0m Request %02d → %s (%ss)\n" \
            "$color" "$id" "$i" "$code" "$time"
    done
}

for ((u=1; u<=USERS; u++)); do
    simulate_user "$u" &
done

wait
echo "=========================================================="
echo "✅ All users finished."
