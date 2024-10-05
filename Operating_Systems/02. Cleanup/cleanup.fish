function cleanup
    #---------Argument setup------------
    argparse 'h/help' 'r/recursive' 'd/dry_run' 'c/confirmation' 's/sort' 'n/inverse' 'i/days_inactive=!_validate_int' 'e/exclude=+' -- $argv

    # Check if directory is given
    if test (count $argv) -eq 0 || test ! -d $argv[1]
        echo "No directory provided! Exiting..."
        return
    end

    set -l target_directory (realpath $argv[1])

    # Categorizing Exclusions
    set directory_exclusions # Directory and filepath exclusions
    set file_exclusions # Filename and filetype exclusions
    for dir in $_flag_exclude
        if string match -q -r "/" $dir
            set -a directory_exclusions (realpath $dir)
        else
            set -a file_exclusions $dir
        end
    end



    #---------Main Function Body------------

    # Get list of all items with a modification date older than $_flag_days_inactive
    if set -q _flag_recursive
        set to_be_deleted (find "$target_directory" -type f -mtime +"$_flag_days_inactive")
    else
        set to_be_deleted (find "$target_directory" -maxdepth 1 -type f -mtime +"$_flag_days_inactive" )
    end

    # Remove files in excluded directories
    for dir in $directory_exclusions
        # Excluded path is folder/directory
        if test -d $dir
            set to_be_deleted (string match -v -r "^($dir.*)" $to_be_deleted)
        # Excluded path is filepath
        else
            set to_be_deleted (string match -v "$dir" $to_be_deleted)
        end
    end

    # Remove files of excluded filename/extension
    for file in $file_exclusions
        set to_be_deleted (string match -v "*$file" $to_be_deleted)
    end

    # Sort remaining files
    if set -q _flag_sort
        # Explanation of command
        # Gets all files | Sorts them (in reverse in flag -n was used)| Gets the parts within quotes
        set to_be_deleted (for file in $to_be_deleted; ls --block-size=K -lQ $file; end | sort -k 5 -h (if set -q  _flag_inverse; echo -r; end) | grep -o '".*"')
    end

    # Dry Run
    if set -q  _flag_dry_run
        echo "Files to be deleted:"
        print_array $to_be_deleted
        return
    end

    # Confirmation
    if set -q  _flag_confirmation
        echo "The following files will be deleted:"
        print_array $to_be_deleted

        read -P 'Are you sure you want to proceed? (y/n)' user_confirm
        if test "$user_confirm" = "y"
            echo "Confirmation received."
        else if test "$user_confirm" = "n"
            echo "Deletion stopped."
            return
        else
            echo "Invalid response, aborting..."
            return
        end
    end   

    # Deletion
    # Function only reaches here if dry_run is false and confimartion has been given
    for file in $to_be_deleted
        echo "Removing $file"
        rm (string trim -c '"\'' $file)
    end
    echo "Files deleted. Exiting."

end


function print_array
    for item in $argv
        echo $item
    end
end