<?php
/**********************************************************************
// Creator: Paul Shipley
// date_:   2019-01-09
// Title:   Import Bank Transactions
// Free software under GNU GPL
***********************************************************************/

class import_bank_transaction
{
    private    $table = "IMPORT_BANK_TRANSACTIONS";

    public $account_code;
    public $filename;
    public $file_sep;
    public $date_format;
    public $date_sep;
    public $trial;

    function __construct()
    {

    }

    function load_transactions($account_code, $filename, $file_sep, $date_format, $date_sep, $trial)
    {
        $this->account_code = $account_code;
        $this->filename = $filename;
        $this->file_sep = $file_sep;
        $this->date_format = $date_format;
        $this->date_sep = $date_sep;
        $this->trial = $trial;

        $num_trans = 0;

        $this->init();
        $this->create_table();
        $num_trans = $this->load_file($this->account_code, $this->filename, $this->file_sep, $this->date_format, $this->date_sep, $this->trial);

        return $num_trans;
    }

    function init()
    {

    }

    function create_table()
    {
        $this->drop_table();

        $sql = "CREATE TABLE ".TB_PREF.$this->table." (id int NOT NULL AUTO_INCREMENT, account_code varchar(15), transdate DATE, transamount DOUBLE, transmemo TINYTEXT, PRIMARY KEY(id))";

        db_query($sql, "Can't create temporary table");
    }

    function drop_table()
    {
        $sql = "DROP TABLE IF EXISTS ".TB_PREF.$this->table;

        db_query($sql, "Can't drop temporary table");
    }

    function add_transaction($bank_account_gl_code, $transdate, $transamount, $transmemo)
    {
        $acct = db_escape($bank_account_gl_code);
        $date = db_escape($transdate);
        $memo = db_escape($transmemo);

        $sql  = "INSERT INTO ".TB_PREF.$this->table." (account_code, transdate , transamount, transmemo) VALUES ($acct, $date, $transamount, $memo)";

        db_query($sql, "Can't create transaction row");
    }

    function load_file($bank_account_gl_code, $filename, $sep, $date_format, $date_sep, $trial)
    {
        $error = false;
        $nextdata = null;
        $line       = 0;
        $entryCount = 0;
        $errCnt     = 0;

        $fp         = @fopen($filename, "r");
        if (!$fp) {
            display_error(_("Error opening file $filename"));
        } else {
            begin_transaction();
            set_time_limit(600); // php maximum execution time
            while (($data = $nextdata) || $line++ == 0) {
                $nextdata = fgetcsv($fp, 4096, $sep);
                if ($data == null) continue;

                list($date, $amt, $memo) = $data;
                str_replace('"', "", $memo);
                $date_std = $this->date2sql($date,$date_format,$date_sep);

                if ($date_std == false) {
                    display_error(_("Error: date '$date' not properly formatted (line $line in import file '{$_FILES['imp']['name']}')"));
                    $error = true;
                }

                if (!$error) {
                    display_notification_centered($line.",".$date_std.",".$amt.",".$memo);

                    $this->add_transaction($bank_account_gl_code,$date_std,$amt,$memo);
                    $entryCount++;
                } else {
                    $errCnt++;
                    $error = false;
                }

            } // where

            if (!$trial) {
                if ($errCnt == 0) {
                    if ($entryCount > 0) {
                        commit_transaction();
                        display_notification_centered(_("$entryCount transactions have been imported."));
                    } else {
                        cancel_transaction();
                        display_error(_("Import file contained no transactions."));
                    }
                } else {
                    cancel_transaction();
                }
            } else {
                // trial load, ignore all transactions
                cancel_transaction();
                $entryCount = 0;
            }

        } // if (!$fp)
        @fclose($fp);

        return $entryCount;
    }

    function get_next_transaction($import_id)
    {
        if ($import_id != 0) {
            $sql = "DELETE FROM ".TB_PREF.$this->table." WHERE id=$import_id";

            db_query($sql, "Can't delete transaction");
        }

        $sql    = "SELECT id, account_code, transdate , transamount, transmemo FROM ".TB_PREF.$this->table." ORDER BY transdate, id LIMIT 1";

        $result = db_query($sql, "Can't get transaction");
        
    	$row = mysqli_fetch_row($result);

        return $row;
    }



    // modified version from includes / date_functions.inc
    function date2sql($date_, $how, $date_sep)
    {
        global $SysPrefs, $tmonths;
        /* takes a date in a the format specified in $DefaultDateFormat
        and converts to a yyyy/mm/dd format */

        //$how = user_date_format();
        //$sep = $SysPrefs->dateseps[user_date_sep()];
        $sep   = $SysPrefs->dateseps[$date_sep];

        $date_ = trim($date_);
        if ($date_ == null || strlen($date_) == 0)
        return "";

        $year = $month= $day  = 0;
        // Split up the date by the separator based on "how" to split it
        if ($how == 0 || $how == 3) // MMDDYYYY or MmmDDYYYY
        list($month, $day, $year) = explode($sep, $date_);
        elseif ($how == 1 || $how == 4) // DDMMYYYY or DDMmYYYY
        list($day, $month, $year) = explode($sep, $date_);
        else // $how == 2 || $how == 5, YYYYMMDD or YYYYMmmDD
        list($year, $month, $day) = explode($sep, $date_);

        if ($year + $day + $month) {
            if ($how > 2) {
                global $tmonths;
                $month = array_search($month, $tmonths);
            }
            //to modify assumption in 2030
            if ($SysPrefs->date_system == 0 || $SysPrefs->date_system == 3) {
                if ((int)$year < 60) {
                    $year = "20".$year;
                }
                elseif ((int)$year > 59 && (int)$year < 100) {
                    $year = "19".$year;
                }
            }
            if ((int)$year > 9999) {
                return 0;
            }
            if ($SysPrefs->date_system == 1)
            list($year, $month, $day) = jalali_to_gregorian($year, $month, $day);
            elseif ($SysPrefs->date_system == 2)
            list($year, $month, $day) = islamic_to_gregorian($year, $month, $day);
        }
        return sprintf("%04d-%02d-%02d", $year, $month, $day);
    }// end of function
}
