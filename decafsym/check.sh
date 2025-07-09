#!/usr/bin/env bash
#
#  compare_dev.sh
#  Compares every  <basename>.out  and  <basename>.err
#  in  references/dev/  against  output/dev/
#  ───────────────────────────────────────────

ref_dir="references/dev"
out_dir="output/dev"

total_cases=0
perfect=0
out_fail=0
err_fail=0
missing=0

printf "\nComparing reference ⇔ output …\n\n"

shopt -s nullglob

for ref_out in "$ref_dir"/*.out; do
    base=$(basename "$ref_out" .out)
    ref_err="$ref_dir/$base.err"
    out_out="$out_dir/$base.out"
    out_err="$out_dir/$base.err"

    (( total_cases++ ))
    status="OK"

    # Quick existence check
    if [[ ! -e $out_out || ! -e $out_err ]]; then
        printf "✗ %-20s  (missing file)\n" "$base"
        (( missing++ ))
        continue
    fi

    # Compare .out
    if ! cmp -s "$ref_out" "$out_out"; then
        status="out-mismatch"
        (( out_fail++ ))
    fi

    # Compare .err
    if ! cmp -s "$ref_err" "$out_err"; then
        if [[ $status == OK ]]; then
            status="err-mismatch"
        else
            status="both-mismatch"
        fi
        (( err_fail++ ))
    fi

    if [[ $status == OK ]]; then
        (( perfect++ ))
    else
        printf "✗ %-20s  (%s)\n" "$base" "$status"
    fi
done

printf "\n──────────────── summary ────────────────\n"
printf " test-cases checked : %3d\n" "$total_cases"
printf " perfect matches    : %3d\n" "$perfect"
printf " out mismatches     : %3d\n" "$out_fail"
printf " err mismatches     : %3d\n" "$err_fail"
printf " missing files      : %3d\n" "$missing"
printf "──────────────────────────────────────────\n"

