#!/bin/bash

function compiler_wrapper_main()
{
    set -e
    local compile_commands_json=
    local source_file=
    local invoke_compiler=true
    while test $# -gt 0
    do
        local shift_by=1
        case "$1" in
        --compile-commands-json)
            compile_commands_json="$2"
            : $((shift_by++))
            source_file=$(realpath "$3")
            : $((shift_by++))
            ;;
        --no-invoke-compiler)
            invoke_compiler=false
            ;;
        --)
            shift
            break
        esac
        shift "$shift_by"
    done
    local remaining_args=( "$@" )
    if test -n "$compile_commands_json"
    then
        # https://clang.llvm.org/docs/JSONCompilationDatabase.html
        local json_entry_directory=
        json_entry_directory=$(dirname "$source_file")
        local json_entry_file=
        json_entry_file=$(basename "$source_file")
        local json_entry_arguments=( "${remaining_args[@]}" )
        json_entry_arguments=( "${json_entry_arguments[@]/%/\"}" )
        json_entry_arguments=( "${json_entry_arguments[@]/#/\"}" )
        json_entry_arguments=( "${json_entry_arguments[@]/%/,}" )
        json_entry_arguments[-1]=${json_entry_arguments[-1]/%,/}

        local json_database_was_empty=false
        if test -s "$compile_commands_json"
        then
            # remove from compilation database
            # 2nd last row '}' since there is no comma
            # and footer ']\n'
            head -n -2 "$compile_commands_json" > temp.txt
            mv temp.txt "$compile_commands_json"
        else
            json_database_was_empty=true
        fi

        {
            if test "$json_database_was_empty" = true
            then
                printf '[\n'
            else
                printf '    },\n'
            fi
            printf '    {\n'
            printf '        "directory": "%s",\n' "$json_entry_directory" 
            printf '        "file": "%s",\n' "$json_entry_file" 
            printf '        "arguments": [%s]\n' "${json_entry_arguments[*]}"
            printf '    }\n'
            printf ']\n'
        } >> "$compile_commands_json"
    fi

    if test "$invoke_compiler" = true
    then
        "${remaining_args[@]}"
    fi
    return 0
}

compiler_wrapper_main "$@"
