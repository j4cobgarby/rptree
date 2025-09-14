_rptree_complete() {
    local cur
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"

    local pids
    pids=$(ps -e -o pid=)

    COMPREPLY=( $(compgen -W "${pids}" -- ${cur}) )
}

complete -F _rptree_complete rptree

