tests_done=0
status=0

pushd build

while :; do
    ((++tests_done))
    cd build; LD_LIBRARY_PATH=. ./fuzz_test; ((status=$?)); echo "Test $tests_done exited with $status";
    echo "$status ======================"
    if [[ $status != 0 ]];
    then
        exit;
    fi;
done

popd build