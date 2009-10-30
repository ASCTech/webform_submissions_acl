<?php
// $Id$

/**
 * @file
 * ACL form.
 *
 * @author Davy Van Den Bremt
 */

function webform_submissions_acl_page(&$form_state, $node) {
  $node_users = webform_submissions_acl_users_by_node($node->nid);
  return theme('webform_submissions_acl_page', drupal_get_form('webform_submissions_acl_form', $node), $node, $node_users);
}

function theme_webform_submissions_acl_page($form, $node, $users) {
  $header = array(t('Username'), t('Operations'));
  $rows = array();

  foreach ($users as $user) {
    $rows[] = array(l($user->name, 'user/'. $user->uid), l(t('Delete'), 'node/'. $node->nid .'/edit/acl/delete/'. $user->uid));
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No users added yet.'), 'colspan' => '2', 'class' => 'message'));
  }

  return $form . theme('table', $header, $rows);
}

function webform_submissions_acl_form(&$form_state, $node) {
  $form = array();

  $form['new_user'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add new user'),
  );
  $form['new_user']['nid'] = array(
    '#type' => 'hidden',
    '#default_value' => $node->nid,
  );
  $form['new_user']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#autocomplete_path' => 'user/autocomplete',
    '#size' => 40,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['new_user']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

function webform_submissions_acl_form_validate($form, &$form_state) {
  $node_users = webform_submissions_acl_users_by_node($form_state['values']['nid'], TRUE);
  $account = user_load(array('name' => $form_state['values']['name']));

  if (in_array($account->uid, $node_users)) {
    form_set_error('name', t('User is already added for this form'));
  }

  $account = user_load(array('name' => $form_state['values']['name']));

  if (!$account) {
    form_set_error('name', t('Invalid user'));
  }
}

function webform_submissions_acl_form_submit($form, &$form_state) {
  $account = user_load(array('name' => $form_state['values']['name']));

  if ($account) {
    db_query("INSERT INTO {webform_submissions_acl} (nid, uid) VALUES (%d, %d)", $form_state['values']['nid'], $account->uid);
    drupal_set_message(t('The user was succesfully added.'), 'status');
  }
  else {
    drupal_set_message(t('Could not add user.'), 'error');
  }
}

/**
 * Menu callback -- Ask for confirmation of acl deletion
 */
function webform_submissions_acl_delete_confirm_form(&$form_state, $node, $account) {
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $account->uid,
  );

  return confirm_form($form,
    t('Are you sure you want to delete access rights for %user from %node?', array('%user' => $account->name, '%node' => $node->title)),
    isset($_GET['destination']) ? $_GET['destination'] : 'node/'. $node->nid .'/edit/acl',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for executing section acl after confirmation
 */
function webform_submissions_acl_delete_confirm_form_submit($form, &$form_state) {
  webform_submissions_acl_delete($form_state['values']['nid'], $form_state['values']['uid']);

  $form_state['redirect'] = 'node/'. $form_state['values']['nid'] .'/edit/acl';
}