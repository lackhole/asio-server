DEPLOY_DIR="$( cd "$( dirname "$0" )" && pwd )"
PORT=`cat "${DEPLOY_DIR}/config.txt" | grep port-dev | cut -d= -f2`

"${DEPLOY_DIR}/build.sh" Debug
"${DEPLOY_DIR}/run.sh" ${PORT}

