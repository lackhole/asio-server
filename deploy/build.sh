DEPLOY_DIR="$( cd "$( dirname "$0" )" && pwd )"
PROJECT_DIR="${DEPLOY_DIR}/.."
BUILD_DIR="${DEPLOY_DIR}/build"

# clear build file
rm -rf "${BUILD_DIR}"
mkdir "${BUILD_DIR}"
cd "${BUILD_DIR}"

BUILD_TYPE=$1
cmake ${PROJECT_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make -j8