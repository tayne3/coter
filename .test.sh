#!/bin/sh

if [ "$(id -u)" != "0" ]; then
    sudo "$0" "$@"
    exit $?
fi

SOURCE_PATH="$1"
SOURCE_DIR=$(dirname ${SOURCE_PATH})
EXEC_NAME=$(basename ${SOURCE_PATH})
EXEC_PATH="/tmp/coter/work"
EXEC_ITERATIONS=10

rm -rf ${EXEC_PATH}
mkdir -p ${EXEC_PATH}
chmod 777 ${EXEC_PATH}
cp ${SOURCE_DIR}/* ${EXEC_PATH} 

echo "0" > /proc/sys/fs/suid_dumpable
echo "${EXEC_PATH}/${SOURCE_FILE}.core" > /proc/sys/kernel/core_pattern
ulimit -c unlimited

echo -n "- core dump level: "
cat /proc/sys/fs/suid_dumpable

echo -n "- core dump path : "   
cat /proc/sys/kernel/core_pattern

echo -n "- core dump limit : "     
ulimit -c

# EXEC_COMMAND="/usr/local/DrMemory-Linux/bin/drmemory -brief -- ${EXEC_PATH}/${EXEC_NAME}"
# EXEC_COMMAND="drmemory -brief -- ${EXEC_PATH}/${EXEC_NAME}"
EXEC_COMMAND="valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -s ${EXEC_PATH}/${EXEC_NAME}"

for i in $(seq 1 ${EXEC_ITERATIONS}); do
    echo "-------------------- Running iteration $i of ${EXEC_ITERATIONS} --------------------"
    ${EXEC_COMMAND}
    if [ $? -ne 0 ]; then
        break
    fi
done
