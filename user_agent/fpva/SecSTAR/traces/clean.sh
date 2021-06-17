source config

echo "=== Cleanup ==="
ssh $SERVER "rm -f /tmp/*.xml" &> /dev/null
ssh $CLIENT "rm -f /tmp/*.xml" &> /dev/null
echo "Done."
