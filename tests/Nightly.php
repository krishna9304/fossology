#!/usr/bin/php
<?php
/***********************************************************
 Copyright (C) 2008 Hewlett-Packard Development Company, L.P.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***********************************************************/

/**
 * Nightly test runs of Top of Trunk
 *
 * @param -h
 * @param [ -p <path>] the path to the fossology sources
 *
 * @return
 *
 * \todo add parameters, e.g. -c for checkout?  -d for run fo-postinstall
 * but these parameters should have a + or - ?  as they are switches for
 * indicating don't do this.... as all actions are on be default.
 *
 * \todo save the fossology log file and start with a fresh one each night.
 *
 * @version "$Id$"
 *
 * Created on Dec 18, 2008
 */

require_once('TestRun.php');
require_once('mailTo.php');

global $mailTo;

$usage = "$argv[0] [-h] [-p <path>]\n" .
         "h: help, this message\n" .
         "p path: the path to the fossology sources to test\n";

$path = NULL;

$options = getopt('hp:');
if(array_key_exists('h', $options))
{
	echo $usage;
	exit(0);
}
if(array_key_exists('p', $options))
{
	$path = $options['p'];
}

/**
 * reportError
 * \brief report the test setup error in a mail message using the global
 * mailTo variable.
 *
 * @param string $error the error string to mail
 * @param string $file either the file path of a file or a string.  If
 * the parameter is a file path to a valid file, then the file will be
 * read and it's contents reported in the mail message.
 */
function reportError($error, $file=NULL)
{
	global $mailTo;

	if(is_file($file))
	{
		if(is_readable($file))
		{
			$longMsg = file_get_contnets($file);
		}
		else
		{
			$longMsg = "$file was not readable\n";
		}
	}
	else if(strlen($file) != 0)
	{
		if(is_string($file))
		{
			$longMsg = $file;
		}
		else
		{
			$longMsg = "Could not append a non-string to the message, " .
			           "reportError was passed an invalid 2nd parameter\n";
		}
	}

	$hdr = "There were errors in the nightly test setup." .
	       "The tests were not run due to one or more errors.\n\n";

	$msg = $hdr . $error . "\n" . $longMsg . "\n";

	// mailx will not take a string as the message body... save to a tmp
	// file and give it that.

	$tmpFile = tempnam('.', 'testError');
	$F = fopen($tmpFile, 'w') or die("Can not open tmp file $tmpFile\n");
	fwrite($F, $msg);
	fclose($F);
	$last = exec("mailx -s 'test Setup Failed' $mailTo < $tmpFile ",$tossme, $rptGen);
}

// Using the standard source path /home/fosstester/fossology

if(array_key_exists('WORKSPACE', $_ENV))
{
	$apath = $_ENV['WORKSPACE'];
	print "workspaces:\napath:$apath\n";
}

$tonight = new TestRun($path);

// Step 1 update sources

print "removing model.dat file so sources will update\n";
$modelPath = '/home/fosstester/fossology/agents/copyright_analysis/model.dat';
//$last = exec("rm $modelPath 2>&1", $output, $rtn);
$last = exec("rm -f $modelPath ", $output, $rtn);

// if the file doesn't exist, that's OK
if((preg_match('/No such file or directory/',$last, $matches)) != 1)
{
	if($rtn != 0)
	{
		$error = "Error, could not remove $modelPath, sources will not update, exiting\n";
		print $error;
		reportError($error,NULL);
		exit(1);
	}
}
print "Updating sources with svn update\n";
if($tonight->svnUpdate() !== TRUE)
{
	$error = "Error, could not svn Update the sources, aborting test\n";
	print $error;
	reportError($error,NULL);
	exit(1);
}

//TODO: remove all log files as sudo

// Step 2 make clean and make sources
print "Making sources\n";
if($tonight->makeSrcs() !== TRUE)
{
	$error = "There were Errors in the make of the sources examine make.out\n";
	print $error;
	reportError($error,'make.out');
	exit(1);
}
//try to stop the scheduler before the make install step.
print "Stopping Scheduler before install\n";
if($tonight->stopScheduler() !== TRUE)
{
	$error = "Could not stop fossology-scheduler, maybe it wasn't running?\n";
	print $error;
	reportError($error, NULL);
}

// Step 4 install fossology
print "Installing fossology\n";
if($tonight->makeInstall() !== TRUE)
{
	$error = "There were Errors in the Installation examine make-install.out\n";
	print $error;
	reportError($error, 'mi.out');
	exit(1);
}

// Step 5 run the post install process
/*
 for most updates you don't have to remake the db and license cache.  Need to
 add a -d for turning it off.
 */

print "Running fo-postinstall\n";
$iRes = $tonight->foPostinstall();
print "install results are:$iRes\n";

if($iRes !== TRUE)
{

	$error = "There were errors in the postinstall process check fop.out\n";
	print $error;
	print "calling reportError\n";
	reportError($error, 'fop.out');
	exit(1);
}

// Step 6 run the scheduler test to make sure everything is clean
print "Starting Scheduler Test\n";
if($tonight->schedulerTest() !== TRUE)
{
	$error = "Error! in scheduler test examine ST.out\n";
	print $error;
	reportError($error, 'ST.out');
	exit(1);
}

print "Starting Scheduler\n";
if($tonight->startScheduler() !== TRUE)
{
	$error = "Error! Could not start fossology-scheduler\n";
	print $error;
	reportError($error, NULL);
	exit(1);
}

print "Running tests\n";
$testPath = "$tonight->srcPath" . "/tests";
print "testpath is:$testPath\n";
if(!chdir($testPath))
{
	$error = "Error can't cd to $testPath\n";
	print $error;
	reportError($error);
	exit(1);
}

print "Running Unit tests\n";

$unitLast = exec('CUnit/runUnitTests.php > CUnit/log-UnitTests 2>&1',
$results, $exitVal);
if($exitVal != 0)
{
	print "FAILURES! There were errors in the Unit tests, examine" .
	      "fossology/tests/Cunit/log-UnitTests\n";
}

print "Running Functional tests\n";
/*
 * This fails if run by fosstester as Db.conf is not world readable and the
 * script is not running a fossy... need to think about this...
 *
 */
$TestLast = exec('./testFOSSology.php -a -e', $results, $rtn);
print "after running tests the output is\n";
print_r($results) . "\n";


/*
 * At this point should have results, generate the results summary and email it.
 *
 * 10-29-2009: the results are generated in testFOSSology.php and mailed there
 * for now.... it should be done here.
 */


/*
 print "Stoping Scheduler\n";
 if($tonight->stopScheduler() !== TRUE)
 {
 print "Error! Could not stop fossology-scheduler\n";
 exit(1);
 }
 */

?>