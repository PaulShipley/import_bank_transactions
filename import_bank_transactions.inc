<?php
/**********************************************************************
Copyright (C) Paul Shipley
Released under the terms of the GNU General Public License, GPL,
as published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License here <http://www.gnu.org/licenses/gpl-3.0.html>.
***********************************************************************/
/**
* Import Bank Transactions module
*
* This is a variation of the gl/gl_bank.php functionality modified to import bank transactions
* Functionality is identical to the orginal, except that the date, bank account, amount and memo fields
* will be pre-populated. The payment/deposit can then be processed as usual. Once committed, the next
* transaction will be shown. Both payments and deposits will be processed in date order, earlist first.
*
* Note that the pre-populated data can be ammended, and the tranaction can even not be processed,
* but this is unlikly to be what you want.
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

        $line       = 0;
        $entryCount = 0;
        $error = false;
        $errCnt     = 0;

        $fp         = @fopen($filename, "r");
        if (!$fp) {
            display_error(_("Error opening file $filename"));
        } else {
            begin_transaction();
            set_time_limit(600); // php maximum execution time
            while ( ($data = fgetcsv($fp, 4096, $file_sep)) !== false ) {
                $line++;
                // ignore blank lines
                if ( $data[0] === null ) {
                    continue;
                }

                list($date, $amt, $memo) = $data;
                str_replace('"', "", $memo);
                $date_std = $this->date2sql($date,$date_format,$date_sep);

                if ($date_std == false) {
                    display_error(_("Error: date '$date' not properly formatted (line $line in import file '{$_FILES['imp']['name']}')"));
                    $error = true;
                }

                if (!$error) {
                    $this->display_screen($line.",".$date_std.",".$amt.",".$memo);
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
                        $this->display_screen(_("$entryCount transactions have been imported."));
                    } else {
                        // no transactions
                        cancel_transaction();
                        $this->display_screen(_("Import file contained no transactions."));
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
     * @param $msg
     * @param $center
     * @return void
     */
    function display_screen($msg, $center=false)
    {
        $style = "style='text-align='" . ($center) ? "centre'" : "left'";
        echo "<span $style class='text'>$msg</span><br/>\n";
    }

    /**
    * date2sql
    *
    * modified version from includes/date_functions.inc
    * 
    * The original version has the date format ($how) and date seperator character ($date_sep)
    * hardcoded to user preferences, which limits the functions usability in siturations
    * where the input date format is not under the users control (eg: file import from third party)
    * 
    * Calling the function without the optional parameters (ie: date2sql($date_)) gives the same
    * result as the original version
    * 
    * Calling the function with the additional parameters allows for greater flexability of
    * date formats (ie: date2sql($date_,2,'-'))
    *
    * @param string  $date_    Date to be converted
    * @param integer $how      Date format (0=MMDDYYYY, 1=DDMMYYYY, 2=YYYYMMDD, 3=MmmDDYYYY, 4=DDMmYYYY, 5=YYYYMmmDD)
    * @param string  $date_sep Date seperator character
    *
    * @return string Date in SQL format (YYYY-MM-DD)
    */
    function date2sql($date_, $how = NULL, $date_sep = NULL)
    {
        global $SysPrefs, $tmonths;
        /* takes a date in a the format specified in $DefaultDateFormat
        and converts to a yyyy/mm/dd format */

        //$how = user_date_format();
        //$sep = $SysPrefs->dateseps[user_date_sep()];
        $how = ($how != NULL ? $how : user_date_format()); 
        $sep = ($date_sep != NULL ? $SysPrefs->dateseps[$date_sep] : $SysPrefs->dateseps[user_date_sep()]);

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
