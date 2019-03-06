<?php
/**
* Import Bank Transactions module
*
* @author Paul Shipley <paul@paulshipley.com.au>
* @copyright 2019 Paul Shipley
* @license GPL
*/
class import_bank_transaction
{
    /**
    *
    * @var string $table Temporary table for holding bank transactions
    *
    */
    private    $table = "IMPORT_BANK_TRANSACTIONS";

    /**
    * Constructor
    *
    * @return NULL
    */
    function __construct()
    {

    }

    /**
    * Load Transactions
    *
    * Load bank transactions into the temporary table
    *
    * @param integer $account_code
    * @param string  $filename
    * @param string  $file_sep
    * @param string  $date_format
    * @param string  $date_sep
    * @param boolean $trial
    *
    * @return integer Number of transactions loaded, will be 0 if only trial or errors found
    */
    function load_transactions($account_code, $filename, $file_sep, $date_format, $date_sep, $trial)
    {
        $this->create_table();

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
                $nextdata = fgetcsv($fp, 4096, $file_sep);
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
                    $this->add_transaction($account_code,$date_std,$amt,$memo);
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
                        // no transactions
                        cancel_transaction();
                        display_error(_("Import file contained no transactions."));
                    }
                } else {
                    // errors found, abandon load
                    cancel_transaction();
                    $entryCount = 0;
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

    /**
    * Create Temporary Table
    *
    * @return NULL
    */
    function create_table()
    {
        $this->drop_table();

        $sql = "CREATE TABLE ".TB_PREF.$this->table." (id int NOT NULL AUTO_INCREMENT, account_code varchar(15), transdate DATE, transamount DOUBLE, transmemo TINYTEXT, PRIMARY KEY(id))";

        db_query($sql, "Can't create temporary table");
    }

    /**
    * Drop Temporary Table
    *
    * @return NULL
    */
    function drop_table()
    {
        $sql = "DROP TABLE IF EXISTS ".TB_PREF.$this->table;

        db_query($sql, "Can't drop temporary table");
    }

    /**
    * Add Transaction
    *
    * Add bank transaction to temporary table to be processed later
    *
    * @param integer  $account_code
    * @param sqldate  $transdate
    * @param double   $transamount
    * @param tinytext $transmemo
    *
    * @return NULL
    */
    function add_transaction($account_code, $transdate, $transamount, $transmemo)
    {
        $acct = db_escape($account_code);
        $date = db_escape($transdate);
        $memo = db_escape($transmemo);

        $sql  = "INSERT INTO ".TB_PREF.$this->table." (account_code, transdate , transamount, transmemo) VALUES ($acct, $date, $transamount, $memo)";

        db_query($sql, "Can't create transaction row");
    }

    /**
    * Delete Transaction
    *
    * Delete transaction from temporary table
    *
    * @param integer $import_id  transaction id to be deleted
    *
    * @return NULL
    */
    function delete_transaction($import_id)
    {
        $sql = "DELETE FROM ".TB_PREF.$this->table." WHERE id=$import_id";

        db_query($sql, "Can't delete transaction");
    }

    /**
    * Get Next Transaction
    *
    * Get next transaction from temporary table
    *
    * @param integer $import_id  previous transaction id to be deleted, skip if 0
    *
    * @return array Contains the selected row
    */
    function get_next_transaction()
    {
        $sql    = "SELECT id, account_code, transdate , transamount, transmemo FROM ".TB_PREF.$this->table." ORDER BY transdate, id LIMIT 1";
        $result = db_query($sql, "Can't get transaction");
        $row    = mysqli_fetch_row($result);

        return $row;
    }

    /**
    * Test if import table has transactions
    *
    * @return boolean  True if table exists and has transactions, else False
    */
    function is_transactions()
    {
        $rc = FALSE;
        $sql    = "SHOW TABLES LIKE '".TB_PREF.$this->table."'";
        $result = db_query($sql,"Can't show tables");
        if (mysqli_num_rows($result) > 0) {
            $sql    = "SELECT count(*) FROM ".TB_PREF.$this->table." LIMIT 1";
            $result = db_query($sql);
            $row    = mysqli_fetch_row($result);

            if ($row[0] > 0) {
                $rc = TRUE;
            }
        }

        return $rc;
    }


    /**
    * date2sql
    *
    * modified version from includes/date_functions.inc
    *
    * @param string  $date_    Date to be converted
    * @param integer $how      Date format (0=MMDDYYYY, 1=DDMMYYYY, 2=YYYYMMDD, 3=MmmDDYYYY, 4=DDMmYYYY, 5=YYYYMmmDD)
    * @param string  $date_sep Date seperator character
    *
    * @return string Date in SQL format (YYYY-MM-DD)
    */
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

        if ($how > 2) {
            global $tmonths;
            $month = array_search($month, $tmonths);
        }

        if ($year + $day + $month) {
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
