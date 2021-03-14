DEPLOY_DIR="$( cd "$( dirname "$0" )" && pwd )"
PORT=`cat "${DEPLOY_DIR}/config.txt" | grep port-prod | cut -d= -f2`

"${DEPLOY_DIR}/build.sh" Release
"${DEPLOY_DIR}/run.sh" ${PORT}

