#!/bin/bash

PATH=$PATH:/bin:/usr/bin:/usr/local/bin:`dirname $0`
export PATH

# Configure the GPS
if [ -e "${RCVW_GPS_INPUT}" ]; then
  configure-ubx.sh &
fi

if [ -z "${PLUGIN_DIR}" ]; then
  PLUGIN_DIR=~plugin
fi

# Enable everything so that a list of all plugins can be obtained
for disabled in ${PLUGIN_DIR}/*/manifest.json.disable; do
  if [ ! -f "${disabled%%.disable}" ]; then
    cat "${disabled}" | envsubst > "${disabled%%.disable}"
  fi
done

tmxctl --list --plugin-dir "${PLUGIN_DIR}" | jq -r '.[] | .name' | sort -u > /tmp/plugins

# The plugin list to run may be set by environment variable.
# If this is empty, it will enable and run everything
INPUT_VAR=`echo ${TMX_COMPONENT}_\\${${TMX_COMPONENT}_COMPONENT}_PLUGINS | envsubst`
PLUGINS=`eval echo \\${${INPUT_VAR}}`
PLUGINS=`tmxctl --list --plugin-dir "${PLUGIN_DIR}" ${PLUGINS} | jq -r '.[] | .name' | sort -u`

# Disable and stop those undesired plugins
echo "${PLUGINS}" | comm -13 - /tmp/plugins | while read plugin; do
  tmxctl --stop "${plugin}" >/dev/null
  tmxctl --list --plugin-dir "${PLUGIN_DIR}" ${plugin} | jq -r 'keys[]+"/manifest.json"' | xargs rm -f
done

# Start the remaining plugins
PLUGINS=`echo "${PLUGINS}" | comm -12 - /tmp/plugins`
for plugin in ${PLUGINS}; do
  # Make this re-runnable by stopping previous executions
  tmxctl --stop "${plugin}" >/dev/null
  tmxctl --start "${plugin}" &
done

wait
exit 0
