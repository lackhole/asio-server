DEPLOY_DIR="$( cd "$( dirname "$0" )" && pwd )"
PROJECT_DIR="${DEPLOY_DIR}/.."
BUILD_DIR="${DEPLOY_DIR}/build"
EXE_DIR="${BUILD_DIR}/asio_server"

PORT=$1
if [ -e "${EXE_DIR}" ]; then
  ${EXE_DIR} ${PORT}
fi

