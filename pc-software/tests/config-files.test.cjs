// Run: node tests/config-files.test.cjs
const fs = require('node:fs');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const path = require('node:path');
const root = path.join(__dirname, '..');
const html = fs.readFileSync(path.join(root, 'frontend/index.html'), 'utf8');
for (const match of html.matchAll(/<script(?:\s[^>]*)?>([\s\S]*?)<\/script>/g)) new Function(match[1]);
const source = fs.readFileSync(path.join(root, 'frontend/config-files.js'), 'utf8');
new Function(source);
const context = {ui: (pt, en) => en};
vm.createContext(context);
vm.runInContext(source.split('const beltProfileFields')[0], context);
const fields = [{n:'x', t:'int', min:1, max:10}, {n:'b', t:'bool'}];
const valid = {version:1, kind:'belt', config:{x:5, b:true}};
assert.equal(context.validateProfile(valid, 'belt', fields).x, 5);
for (const invalid of [null, [], {...valid, kind:'motors'}, {...valid, version:2},
  {...valid, config:{x:11, b:true}}, {...valid, config:{x:0, b:true}},
  {...valid, config:{x:1.1, b:true}}, {...valid, config:{x:5, b:1}},
  {...valid, config:{x:5, b:true, align_x_offset:2}}, {...valid, config:{b:true}},
  {...valid, config:{x:'5', b:true}}, {...valid, config:{x:NaN, b:true}},
  {...valid, config:{x:Infinity, b:true}}]) {
  assert.throws(() => context.validateProfile(invalid, 'belt', fields));
}
const rampFields = ['brk_ov_start', 'brk_ov_end'].map(n => ({n, t:'float', min:1, max:100}));
assert.throws(() => context.validateProfile({version:1, kind:'motors',
  config:{brk_ov_start:30, brk_ov_end:29}}, 'motors', rampFields));
assert.equal(context.validateProfile({version:1, kind:'motors',
  config:{brk_ov_start:30, brk_ov_end:32}}, 'motors', rampFields).brk_ov_end, 32);
assert.ok(html.includes('/static/config-files.js'));
for (const id of ['motor-export', 'motor-import', 'motor-import-file', 'belt-export', 'belt-import', 'belt-import-file'])
  assert.ok(html.includes(`id="${id}"`));
console.log('PASS: JavaScript syntax, profile validation, ramp consistency and UI hooks');
