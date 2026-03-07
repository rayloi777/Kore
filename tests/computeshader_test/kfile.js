const project = new Project('computeshader_test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');

project.flatten();

resolve(project);
