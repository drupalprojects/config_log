<?php



/**
 * Implements hook_drush_command().
 *
 * In this hook, you specify which commands your drush module makes available,
 * what it does and description.
 *
 * Notice how this structure closely resembles how you define menu hooks.
 *
 * See `drush topic docs-commands` for a list of recognized keys.
 */
function config_log_drush_command() {
  $items = array();
  $items['config-log-commit'] = array(
    'description' => "Retrieve and commit config changes (demo only).",
    'arguments' => array(
      'repo path' => 'Path to the git repo.',
    ),
    'options' => array(
      'config-vcs' => 'Dirctory with the config (defaults to config_vcs)',
    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_config_log_commit($path = '') {
  if (!$path) {
    return drush_log('Repo path is required', 'error');
  }
  $path = realpath($path);
  $config_vcs = $path . '/' . drush_get_option('config-vcs', 'config_vcs');
  $track = "$path/config_log_commit_last";
  $last = (int) trim(@file_get_contents($track));
  do {
    $result = db_query('SELECT * FROM {config_log} WHERE clid > :id ORDER BY clid ASC', array('id' => $last));
    foreach ($result as $row) {
      $account = user_load($row->uid);
      $filename = $config_vcs . '/' . $row->name . '.yml';
      file_put_contents($filename, $row->data);
      $cmd = 'cd ' . escapeshellarg($config_vcs) . ' && git add * && (git diff --quiet HEAD || git commit -m ' .  escapeshellarg($row->clid . ' user: ' . $account->getUsername() . ' operation: ' . $row->operation . ' name:' . $row->name).')';
      $out = system($cmd, $retval);
      if ($retval == 0) {
        file_put_contents($track, $row->clid);
        $last = $row->clid;
      }
      else {
        throw new Exception($out);
      }
    }
    sleep(5);
  } while (1);
}